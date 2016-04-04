/*
 * BTreeFile.cc - implementation of methods of class BTree::File
 * as declared in BTreeFile.h
 *
 * Copyright (c) 2001, 2002, 2003, 2013 - Russell C. Bjork
 * Trivial changes 2016 - Russ Tuck
 */

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <string.h>
using std::cerr;

#include "BTreeFile.h"
#include "BTreeBlock.h"

/* The following private class represents the way data is physically stored
 * on the disk.	 Physical blocks are numbered 1 .. file size.
 *
 * The header is always stored in block 1 - with child 0 being the
 * the number of the root block, child 1 the number of blocks used,
 * and child 2 the first free block.  In the header block, _numberOfKeys
 * is the degree of the tree.
 *
 * In a free block, child 0 points to the next block on the free list, and
 * the remaining data is unused.
 *
 */

class BTreeFile::PhysicalBlock
{
    friend class BTreeFile;

  private:

    static const unsigned KEY_LENGTH = 16;
    static const unsigned VALUE_LENGTH = 32;
    static const unsigned PHYSICAL_BLOCK_SIZE = 512;

    PhysicalBlock()
        : _numberOfKeys(0)
        {
            for (int i = 0; i < DEGREE; i ++)
                _childData[i] = 0;
            for (int i = 0; i < KEY_LENGTH * (DEGREE - 1); i ++)
                _keyData[i] = '\0';
            for (int i = 0; i < VALUE_LENGTH * (DEGREE - 1); i ++)
                _valueData[i] = '\0';
            for (int i = 0; i < sizeof(_filler); i ++)
                _filler[i] = '\0';
        }

    unsigned _numberOfKeys;
    BlockNumber _childData[DEGREE];
    char _keyData[KEY_LENGTH * (DEGREE-1)];
    char _valueData[VALUE_LENGTH * (DEGREE-1)];

    char _filler [ PHYSICAL_BLOCK_SIZE
                   - sizeof(unsigned)
                   - sizeof(BlockNumber) * DEGREE
                   - sizeof(char) * (KEY_LENGTH * (DEGREE - 1))
                   - sizeof(char) * (VALUE_LENGTH * (DEGREE - 1)) ];
};

BTreeFile::BTreeFile(string name)
    : _name(name), _totalGets(0), _totalPuts(0), _headerGets(0), _headerPuts(0),
      _freeGets(0), _freePuts(0)
{
    // Open or create the file

    _stream = fopen(name.c_str(), "r+b"); // Open an existing file
    if (_stream != NULL) {
        // Opened an existing file - read its header

        PhysicalBlock header;
        if (! getBlock(1, header)) {
            cerr << "Error reading header block" << endl;
            cerr << "At line " << __LINE__ << " in file " << __FILE__ << endl;
            exit(1);
        }

        _headerGets ++;
        _degree = header._numberOfKeys;
        _root = header._childData[0];
        _lastUsed = header._childData[1];
        _free = header._childData[2];

        if (_degree != DEGREE) {
            cerr << "Degree recorded in header (" << _degree <<
                ") does not match degree expected by program (" <<
                DEGREE << ")" << endl;
            cerr << "At line " << __LINE__ << " in file " << __FILE__ << endl;
            exit(1);
        }

        cout << "Opened file ";
        printHeaderInfo();

    } else {

        _stream = fopen(name.c_str(), "w+b"); // Create a new file
        if (_stream != NULL) {
            _degree = DEGREE;
            _root = 0;
            _lastUsed = 1;
            _free = 0;
            updateHeader();

            cout << "Created file ";
            printHeaderInfo();
        } else {
            cerr << "Unable to open or create" << name << endl;
            cerr << "At line " << __LINE__ << " in file " << __FILE__ << endl;
            exit(1);
        }
    }
}

string BTreeFile::getName() const
{
    return _name;
}

BTreeFile::BlockNumber BTreeFile::getRoot() const
{
    return _root;
}

void BTreeFile::setRoot(BlockNumber root)
{
    _root = root;

    // Reflect the change in the header on disk

    updateHeader();
}

bool BTreeFile::getBlock(BlockNumber number, BTreeBlock & block)
{
    PhysicalBlock buffer;

    // Actually get the data

    if (! getBlock(number, buffer))
        return false;

    // Copy information in the buffer to parameters, changing the format
    // of the strings to C++ strings

    block.setNumberOfKeys(buffer._numberOfKeys);

    char key[PhysicalBlock::KEY_LENGTH + 1];
    char value[PhysicalBlock::VALUE_LENGTH + 1];

    for (unsigned i = 0; i < buffer._numberOfKeys; i ++) {
        strncpy(key, buffer._keyData + i * PhysicalBlock::KEY_LENGTH,
                PhysicalBlock::KEY_LENGTH);
        strncpy(value, buffer._valueData + i * PhysicalBlock::VALUE_LENGTH,
                PhysicalBlock::VALUE_LENGTH);
        block.setKey(i, key);
        block.setValue(i, value);
        block.setChild(i, buffer._childData[i]);
    }

    // One more children than keys/data

    block.setChild(block.getNumberOfKeys(),
                   buffer._childData[buffer._numberOfKeys]);

    return true;
}

bool BTreeFile::putBlock(BlockNumber number, const BTreeBlock & block)
{
    PhysicalBlock buffer;

    // Copy information in parameters to disk buffer, changing the format
    // of the strings from C++ strings

    buffer._numberOfKeys = block.getNumberOfKeys();

    for (unsigned i = 0; i < buffer._numberOfKeys; i ++) {
        strncpy(buffer._keyData + i * PhysicalBlock::KEY_LENGTH,
                block.getKey(i).c_str(), PhysicalBlock::KEY_LENGTH);
        strncpy(buffer._valueData + i * PhysicalBlock::VALUE_LENGTH,
                block.getValue(i).c_str(), PhysicalBlock::VALUE_LENGTH);
        buffer._childData[i] = block.getChild(i);
    }

    // One more children than keys/data

    buffer._childData[buffer._numberOfKeys] =
        block.getChild(buffer._numberOfKeys);

    // Actually transfer data to disk

    return putBlock(number, buffer);
}

BTreeFile::BlockNumber BTreeFile::allocateBlock()
{
    BlockNumber allocated;

    if (_free != 0) {
        // Allocate a block from the free list

        allocated = _free;

        // Update _free to point to next block on free list

        PhysicalBlock allocatedBlock;
        if (! getBlock(allocated, allocatedBlock)) {
            cerr << "Unable to read supposedly free block "
                 << allocated << endl;
            cerr << "At line " << __LINE__ << " in file " << __FILE__ << endl;
            exit(1);
        }
        _freeGets ++;
        _free = allocatedBlock._childData[0];
    } else {
        // Extend the file by one block

        allocated = ++ _lastUsed;
    }

    // Either way, the changes must be reflected in the copy of the
    // header on disk

    updateHeader();

    return allocated;
}

void BTreeFile::deallocateBlock(BlockNumber number)
{
    // Link this block into the free list

    PhysicalBlock deallocatedBlock;
    deallocatedBlock._numberOfKeys = 0;
    deallocatedBlock._childData[0] = _free;
    if (! putBlock(number, deallocatedBlock)) {
        cerr << "Unable to put free block " << number << endl;
        cerr << "At line " << __LINE__ << " in file " << __FILE__ << endl;
        exit(1);
    }
    _freePuts ++;
    _free = number;

    // Reflect changes in the header on disk

    updateHeader();
}

void BTreeFile::printHeaderInfo() const
{
    cout << _name << " of degree " << _degree
         << " using " << _lastUsed << " blocks. Root at " << _root
         << ", first free " << _free << endl;
}

void BTreeFile::printBlock(BlockNumber number, bool recursive, unsigned level)
{
    // Get information in the block

    BTreeBlock block;
    if (getBlock(number, block)) {
        // Print out information in the block

        if (recursive) {
            cout << setw(level * 2) << "" << "Block " << number <<
                " at level " << level;
        } else {
            cout << "Block " << number;
        }
        cout << " contains " << block.getNumberOfKeys() << " key(s)" << endl;

        for (unsigned i = 0; i <= block.getNumberOfKeys(); i ++) {
            if (recursive)
                cout << setw(level * 2) << "";
            cout << setw(4) << block.getChild(i);
            if (i < block.getNumberOfKeys())
                cout << "    " << setw(PhysicalBlock::KEY_LENGTH)
                     << block.getKey(i)
                     << "    " << setw(PhysicalBlock::VALUE_LENGTH)
                     << block.getValue(i);
            cout << endl;
        }
        cout << endl;
    } else {
        cerr << "Unable to get block " << number << endl;
        cerr << "At line " << __LINE__ << " in file " << __FILE__ << endl;
        exit(1);
    }

    // If recursive and block is not a leaf, print out its children.  Since all
    // leaves are at the same level, it if has one valid child, all children are
    // valid

    if (recursive && block.getChild(0) != 0) {
        for (unsigned i = 0; i <= block.getNumberOfKeys(); i ++) {
            printBlock(block.getChild(i), recursive, level + 1);
        }
    }
}

BTreeFile::~BTreeFile()
{
    cout << "Closing file ";
    printHeaderInfo();
    cout << "Total gets done: " << _totalGets << " (" << _headerGets
         << " header, " << _freeGets << " free blocks)." << endl
         << "Total puts done: " << _totalPuts << " (" << _headerPuts
         << " header, " << _freePuts << " free blocks)." << endl;

    fclose(_stream);
}

bool BTreeFile::getBlock(BlockNumber number, PhysicalBlock & buffer)
{
    _totalGets ++;
    fseek(_stream, PhysicalBlock::PHYSICAL_BLOCK_SIZE * (number - 1), SEEK_SET);
    return fread((void *) & buffer, sizeof(PhysicalBlock), 1, _stream) == 1;
}

bool BTreeFile::putBlock(BlockNumber number, const PhysicalBlock & buffer)
{
    _totalPuts ++;
    fseek(_stream, PhysicalBlock::PHYSICAL_BLOCK_SIZE * (number - 1), SEEK_SET);
    return fwrite((const void *) & buffer,
                  sizeof(PhysicalBlock), 1, _stream) == 1;
}

void BTreeFile::updateHeader()
{
    PhysicalBlock header;
    header._numberOfKeys = _degree;
    header._childData[0] = _root;
    header._childData[1] = _lastUsed;
    header._childData[2] = _free;
    if (! putBlock(1, header)) {
        cerr << "Unable to put header block" << endl;
        cerr << "At line " << __LINE__ << " in file " << __FILE__ << endl;
        exit(1);
    }
    _headerPuts ++;
}

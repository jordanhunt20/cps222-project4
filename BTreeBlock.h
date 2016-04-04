/*
 * BTreeBlock.h - declaration of the class BTreeBlock, used as part
 * of actually implementing a B Tree.  CPS222 project 4
 *
 * Copyright (c) 2001, 2002, 2003, 2015 - Russell C. Bjork
 * Trivial changes 2016 - Russ Tuck
 */

#ifndef BTREE_BLOCK_H
#define BTREE_BLOCK_H

#include <string>
#include "BTreeFile.h"

/* An object of class BTreeBlock is used to hold an in-memory copy of
 * one block of the BTree.  The method BTreeFile::getBlock() is used to
 * copy a block from the disk file to a Block object; and the method
 * BTreeFile::putBlock is used to copy a Block object back to the file.
 *
 * Although a block on disk can hold only DEGREE - 1 keys and associated
 * values, and can store DEGREE block numbers of child blocks,
 * an in-memory copy is allowed to hold one more key, value, and child than
 * this to facilitate splitting.  Of course, an over-full in-memory copy will
 * never be read as such from disk, nor can it be transferred to disk before
 * it is split.
 */

class BTreeBlock
{
  public:

    // Constructor - create an empty block
    BTreeBlock();

    // Test to see whether this is a leaf block - return true iff it is
    bool isLeaf() const;

    // Accessors for the various instance variables.
    // Parameter: index - index into array of keys, values, children
    // For getKey() and getValue() parameters, valid values of index lie
    //   in the range 0 .. number of keys - 1
    // For getChild(), index can be as big as number of keys, since there is
    //   always one more child
    unsigned getNumberOfKeys() const;
    string getKey(unsigned index) const;
    string getValue(unsigned index) const;
    BTreeFile::BlockNumber getChild(unsigned index) const;

    // Mutators for the various instance variables
    // Parameter: index - index into array of keys, values, children
    //  (same constraints on value of index as for get___ methods)
    // Parameter: key/value/child - new value to set in specified slot
    void setNumberOfKeys(unsigned numberOfKeys);
    void setKey(unsigned index, string key);
    void setValue(unsigned index, string value);
    void setChild(unsigned index, BTreeFile::BlockNumber child);

    // Find the position where a given key is / belongs
    // Parameter: key - the key whose position is to be found
    // Returns:
    //   If the key occurs in the block, its index.
    //   If it does not occur, where it would belong. (If it is equal to the
    //     number of keys, this indicates it is greater than any key
    //     currently in the block.)
    //   If the block has children, this value is the index of the child to
    //     look for it in.
    int getPosition(string key);

    // Insert a new key, value, and associated child into the block,
    // pushing existing entries to the right if need be, and
    // incrementing number of keys Parameter: position - position
    // where the new key and value go; the new child will go one to
    // right of this.  Parameters: key, value, child - the new entry
    // to insert. Child will go to the right of key and value
    void insert(int position, string key, string value, BTreeFile::BlockNumber child);

    // Test to see whether this block needs to be split following an insert.
    // A split will be needed if the block now contains too many keys.
    bool splitNeeded() const;

    // Split this block in two.  This should be done _after_ an
    // insertion that results in the block becoming overfull, so that
    // the newly-inserted entry ends up in the correct half.  The left
    // half of the original block is left in place, but the count of
    // the number of keys is reduced; the right half of the original
    // block is copied to a new block, and the number of keys in that
    // half is set appropriately.  Parameters: (all are reference
    // parameters that are set by this method) promotedKey,
    // promotedValue - information that should be inserted into the
    // parent block (or a new root) rightHalf - block into which half
    // the information is place
    void split(string & promotedKey, string & promotedValue,
               BTreeBlock & rightHalf);

  private:

    unsigned _numberOfKeys;
    string _key[DEGREE];
    string _value[DEGREE];
    BTreeFile::BlockNumber _child[DEGREE + 1];
};

# endif

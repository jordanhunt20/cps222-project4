/*
 * BTree.cc - implementation of methods of class BTree as declared in BTree.h.
 *
 * Several of the methods in this file must be implemented by students for
 * cs222 project 4
 *
 * If this file is compiled with the symbol PROFESSOR_VERSION defined,
 * it expects a file of that name to exist, and uses the code from
 * that file in place of selected dummy methods left to be written by students.
 *
 * Copyright (c) 2001, 2002, 2003 - Russell C. Bjork
 * Trivial changes 2016 - Russ Tuck
 * Modified by Jordan Hunt and Adam Vigneaux
 */

#include <iostream>
using std::cout;

#include "BTree.h"
#include "BTreeFile.h"
#include "BTreeBlock.h"

BTree::BTree(string name)
: _file(* new BTreeFile(name))
{ }

#ifndef PROFESSOR_VERSION


/*
 * Recursive auxiliary function for lookup by key
 * @param1 string specifying key of key-value pair to be found
 * @param2 BTreeBlock number of root to start serach from
*/
bool BTree::privateLookup(string key, BTreeFile::BlockNumber numRoot, string & value) const
{
    cout << std::endl << "here" << std::endl;
    // top of the part of the tree still needed to be searched
    BTreeBlock top;
    // set root equal to the root of the tree
    _file.getBlock(numRoot, top);

    //get number of keys in root
    int numKeys = top.getNumberOfKeys();

    // increment i until key (from parameter) is greater than
    // the corresponding key in the root
    int i = 0;
    while (i < numKeys && top.getKey(i) < key) i++;
    if (i < numKeys && top.getKey(i) == key) {
        value = top.getValue(i);
        return true;
    } else {
        if (top.isLeaf()) return false;
        BTreeFile::BlockNumber childNum;
        childNum = top.getChild(i);
        bool found = privateLookup(key, childNum, value);
        return found;
    }
}

void BTree::insert(string key, string value)
{
    // Student code goes here - remove this line
}

bool BTree::lookup(string key, string & value) const
{
    cout << std::endl << 99 << std::endl;
    BTreeBlock root;
    // BTreeFile::getRoot() returns the block number of the root
    // for the file
    BTreeFile::BlockNumber numRoot = _file.getRoot();
    cout << std::endl << 3 << std::endl;
    // set root equal to the root of the tree
    _file.getBlock(numRoot, root);
    cout << std::endl << 91 << std::endl;
    //get number of keys in root
    int numKeys = root.getNumberOfKeys();
    cout << std::endl << numKeys << std::endl;
    // increment i until key (from parameter) is greater than
    // the corresponding key in the root
    int i = 0;
    while (i < numKeys && root.getKey(i) < key) i++;
    if (i < numKeys && root.getKey(i) == key) {
        value = root.getValue(i);
        cout << 1 << std::endl;
        cout << std::endl << "here" << std::endl;
        return true;
    } else {
        cout << std::endl << "here 2" << std::endl;
        BTreeFile::BlockNumber childNum;
        childNum = root.getChild(i);
        bool found = privateLookup(key, childNum, value);
        cout << 2 << std::endl;
        return found;
    }
}

bool BTree::remove(string key)
{
    return false; // Student code goes here - remove this line
}

#else

#define QUOTE(Q) #Q
#define INCLUDE_NAME(X) QUOTE(X)
#include INCLUDE_NAME(PROFESSOR_VERSION)

#endif

void BTree::print() const
{
    cout << "BTree in file ";
    _file.printHeaderInfo();
    cout << endl;

    BTreeFile::BlockNumber root = _file.getRoot();
    if (root == 0)
        cout << "Empty tree" << endl;
    else
        _file.printBlock(root, true, 1);
}

void BTree::print(BTreeFile::BlockNumber blockNumber) const
{
    _file.printBlock(blockNumber, false, 1);
}

BTree::~BTree()
{
    delete (& _file);
}

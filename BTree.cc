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
using std::endl;

#include "BTree.h"
#include "BTreeFile.h"
#include "BTreeBlock.h"
#include <cmath>

BTree::BTree(string name)
: _file(* new BTreeFile(name))
{ }

#ifndef PROFESSOR_VERSION


/*
 * Recursive auxiliary function for lookup by key
 * @param1 string specifying key of key-value pair to be found
 * @param2 BTreeBlock number of root to start serach from
 * @return true if found, false if not
*/
bool BTree::privateLookup(string key, BTreeFile::BlockNumber numRoot, string & value) const
{
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

/*
bool BTree::findForInsert(string key,
                            BTreeBlock & top,
                            BTreeBlock & blockToInsert) const
{


    //get number of keys in root
    int numKeys = top.getNumberOfKeys();

    // increment i until key (from parameter) is greater than
    // the corresponding key in the root
    int i = 0;
    while (i < numKeys && top.getKey(i) < key) i++;
    if (i < numKeys && top.getKey(i) == key) {
        blockToInsert = top;
        return true;
    } else {
        if (top.isLeaf()) {
            blockToInsert = top;
            return false;
        }
        BTreeFile::BlockNumber childNum;
        childNum = top.getChild(i);
        BTreeBlock child;
        _file.getBlock(childNum, child);
        bool found = findForInsert(key, child, blockToInsert);
        return found;
    }
}
*/
void BTree::insert(string key, string value)
{
    // getRoot() returns the block number of the root
    BTreeFile::BlockNumber numCurr = _file.getRoot();

    // start with curr equal to the root of the tree
    BTreeBlock curr;
    _file.getBlock(numCurr, curr);

    // find position the key parameter should go to in curr
    int position = curr.getPosition(key);
    cout << "curr[0]: " << curr.getKey(0) << endl;
    cout << "curr[1]: " << curr.getKey(1) << endl;
    cout << "numCurr" << numCurr << endl;
    cout << "1 position: " << position << endl;

    // empty tree if root = 0
    if(numCurr == 0) {
        numCurr = _file.allocateBlock();
        _file.getBlock(numCurr, curr);
        _file.setRoot(numCurr);
        curr.setChild(position, 0);
    }

    // holds curr's parent's number
    BTreeFile::BlockNumber numParent = 0; // default

    // holds number of curr's child at position
    BTreeFile::BlockNumber numChild = curr.getChild(position);

    // follow the tree down until it reaches a leaf
    while(!curr.isLeaf()) {
        cout << "num curr: " << numCurr << endl;
        cout << "2 Position: " << position << endl;
        // set numParent to numCurr
        numParent = numCurr;

        // set numCurr to numChild
        numCurr = numChild;

        // set curr to block at numChild
        _file.getBlock(numCurr, curr);

        // set position to position for this key at the child
        position = curr.getPosition(key);

        // set child to the child of the child
        numChild = curr.getChild(position);
    }
    position = curr.getPosition(key);
    cout << "before insert: " << endl;
    cout << "position: " << position << endl;

    // insert key into curr at position with numChild to its right
    curr.insert(position, key, value, numChild);

    // if no split is needed, write curr to disk
    if(!curr.splitNeeded()) _file.putBlock(numCurr, curr);

    // follow curr upwards as long as a split is needed, and do
    // the necessary operations to each parent
    while (curr.splitNeeded())
    {
        // new block for right half of curr
        BTreeBlock rightChild;

        // variables to hold the promoted key and promoted value
        string promotedKey;
        string promotedValue;

        // split curr; split() mutates all its parameters, along with
        // its caller (in this case curr)
        curr.split(promotedKey, promotedValue, rightChild);

        // write curr (now the left child of parent) to disk
        _file.putBlock(numCurr, curr);

        // get the number of an available block
        BTreeFile::BlockNumber numRightChild = _file.allocateBlock();

        // write the right file to disk
        _file.putBlock(numRightChild, rightChild);

        // if no parent, curr is root, so create new root
        if (numParent == 0) {
            numParent = _file.allocateBlock();
            _file.setRoot(numParent);
        }

        // set parent equal to the parent of curr
        BTreeBlock parent;
        _file.getBlock(numParent, parent);

        position = parent.getPosition(promotedKey);

        // insert promoted key and value into parent; assigns right child
        parent.insert(position, promotedKey, promotedValue, numRightChild);

        // set left child
        parent.setChild(position, numCurr);

        // write parent to disk
        _file.putBlock(numParent, parent);

        curr = parent;
        numCurr = numParent;
    }
}
/*
* Mutator
* finds a value corresponding to a key in the tree, sets it to value
* @param1 string key of key value pair to finds
* @param2 string & string to mutate to the desired value
* @return true if found, false if not
*/
bool BTree::lookup(string key, string & value) const
{
    BTreeBlock root;
    // BTreeFile::getRoot() returns the block number of the root
    // for the file
    BTreeFile::BlockNumber numRoot = _file.getRoot();
    // set root equal to the root of the tree
    _file.getBlock(numRoot, root);
    //get number of keys in root
    int numKeys = root.getNumberOfKeys();
    // increment i until key (from parameter) is greater than
    // the corresponding key in the root
    int i = 0;
    while (i < numKeys && root.getKey(i) < key) i++;
    if (i < numKeys && root.getKey(i) == key) {
        value = root.getValue(i);
        return true;
    } else {
        BTreeFile::BlockNumber childNum;
        childNum = root.getChild(i);
        bool found = privateLookup(key, childNum, value);
        return found;
    }
}

bool BTree::remove(string key)
{
    // getRoot() returns the block number of the root
    BTreeFile::BlockNumber numCurr = _file.getRoot();

    // if empty tree return 0 (not found)
    if (numCurr == 0) {
        return false;
    }

    // will hold the position in the tree to follow
       int position;

       bool notFound = true;

       // will hold the number of the parent of curr. set to default 0
       BTreeFile::BlockNumber numParent = 0;

       // will hold the number of the child of curr at position
       BTreeFile::BlockNumber numChild;

       // will hold the height of the tree
       int height = 1;

       // set curr equal to the block with number numCurr
       BTreeBlock curr;
       _file.getBlock(numCurr, curr); // mutates curr

       // follow the tree down until key is found or
       // until bottom of tree is reached
       while(notFound && curr.getChild(position) != 0) {



           // find position the key parameter should go to in curr
           position = curr.getPosition(key);

           // holds number of curr's child at position
           numChild = curr.getChild(position);

           // if the key in curr at position is the key we are looking for,
           // then we are done looking
           if (curr.getKey(position) == key) {
               notFound = false;
           } else {

               // set numParent to numCurr
               numParent = numCurr;

               // set numCurr to numChild
               numCurr = numChild;

               // set curr to block at numChild
               _file.getBlock(numCurr, curr);

               // set position to position for this key at the child
               position = curr.getPosition(key);

               // set child to the child of the child
               numChild = curr.getChild(position);

               // increment height
               height++;
           }
       }

       // temp used to go the rest of the way to the bottom of the tree
       // to get the height of the tree
       BTreeBlock temp;
       BTreeFile::BlockNumber numTemp;
       temp = curr;
       while (temp.getChild(0) != 0) {
           numTemp = temp.getChild(0);
           _file.getBlock(numTemp, temp);
           height++;
       }

       cout << endl;
       cout << "Height: " << height << endl;
       cout << "currNumber: " << numCurr << endl;

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

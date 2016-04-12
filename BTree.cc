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
#include <stack>

BTree::BTree(string name)
: _file(* new BTreeFile(name))
{ }

#ifndef PROFESSOR_VERSION


/*
 * Recursive auxiliary function for finding a block containing a key
 * @param1 string specifying key to be found
 * @param2 BTreeBlock number of root to start serach from
 * @return true if found, false if not
*/
bool BTree::find(string key, BTreeFile::BlockNumber & numCurr, std::stack<BTreeFile::BlockNumber> & path ) const
{
    // BTreeBlock to hold the block specified by numCurr
    BTreeBlock curr;

    // set curr equal to the block specified by numCurr
    _file.getBlock(numCurr, curr);

    // position either of the key if found, or of the child to
    // search in
    int position = curr.getPosition(key);

    if ( curr.getKey(position) == key ) { // if the key is in curr return true
        return true;
    } else if ( curr.isLeaf() ) { // if curr is a leaf, return false
        return false;
    } else { // if the key is not in curr and curr isn't a leaf, check curr's child
        // add numCurr to the path
        path.push(numCurr);

        // set numCurr equal to curr's child so we can continue following the path
        // to the block containing the key
        numCurr = curr.getChild( position );

        // recursive call to find on the child of the block
        // that called this function
        return find( key, numCurr, path );
    }
}

/*
 * Inserts a key value pair into the binary tree
 * @param1 string: the key to add
 * @param2 string: the value to add
*/
void BTree::insert(string key, string value)
{
    // getRoot() returns the block number of the root
    BTreeFile::BlockNumber numCurr = _file.getRoot();

    // curr will hold hold the blocks of the tree we will change
    BTreeBlock curr;

    // holds path to curr
    std::stack<BTreeFile::BlockNumber> path;

    // holds number of curr's parent
    BTreeFile::BlockNumber numParent;

    // holds number of curr's child at position
    BTreeFile::BlockNumber numChild = 0;

    // default false
    bool found = false;

    // empty tree if root = 0
    if(numCurr == 0) {
        // allocate a block to hold the new key value pair
        numCurr = _file.allocateBlock();

        // set curr equal to the new block
        _file.getBlock(numCurr, curr);

        // set the root of the BTree to curr
        _file.setRoot(numCurr);

        // set curr's child equal to 0
        curr.setChild(0, 0);
    } else {
        // set curr equal to the block at numCurr
        _file.getBlock(numCurr, curr);


        // find returns true if the key is found, and false if not
        // it also sets numCurr equal to the last block checked,
        // and numParent equal to the number of numCurr's parent
        found = find ( key, numCurr, path );

        // set curr equal to the block at numCurr
        _file.getBlock(numCurr, curr);
    }

    // find position the key parameter should go to in curr
    BTreeFile::BlockNumber position = curr.getPosition(key);

    if ( found ) {
        curr.setValue( position, value );
        _file.putBlock( numCurr, curr );
    } else {
        // insert key into curr at position with numChild to its right
        curr.insert(position, key, value, numChild);

        // if no split is needed, write curr to disk
        if(!curr.splitNeeded()) {
            _file.putBlock(numCurr, curr);
        }

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
            if ( path.empty()) {
                numParent = _file.allocateBlock();
                _file.setRoot(numParent);
            } else {
                // set numParent equal to the top of the path stack
                numParent = path.top();
                path.pop();
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
}

/*
* Mutator
* finds a value corresponding to a key in the tree, sets it to value
* @param1 string: key of key value pair to finds
* @param2 string & string: value to mutate to the desired value
* @return bool: true if found, false if not
*/
bool BTree::lookup(string key, string & value) const
{
    // will hold the root of the BTree
    BTreeBlock root;

    // BTreeFile::getRoot() returns the block number of the root
    // for the file
    BTreeFile::BlockNumber numRoot = _file.getRoot();

    // set root equal to the root of the tree
    _file.getBlock(numRoot, root);

    // will not be used, only necessary for call to parent
    std::stack<BTreeFile::BlockNumber> path;

    // call find on the key with numRoot
    // find returns true if it finds it, and sets
    // numRoot equal to the last block checked
    if( find ( key, numRoot, path ) ) {

        // will hold the block where the key was found
        BTreeBlock blockFound;

        // set blockFound equal to the block where the key was found
        _file.getBlock(numRoot, blockFound);

        // the position containing the key in blockFound
        int position = blockFound.getPosition(key);

        // set value equal to the value at position in blockFound
        value = blockFound.getValue(position);

        // return true since the value was found
        return true;
    } else {
        // return false since the key was not found in the BTree
        return false;
    }
}


/*
 * returns whether or not the block needs more keys
 * @return bool: true if key needed, false if not
 */
bool BTree::needsKeys( BTreeFile::BlockNumber numBlock, BTreeBlock block ) const
{
    int minNumKeys;
    if (!isRoot(numBlock)) {
        minNumKeys = ceil ( DEGREE / 2.0 ) - 1;
    } else {
        minNumKeys = 1;
    }
    cout << "minNumKeys: " << minNumKeys << " for block: " << numBlock << endl;
    cout << "Which has: " << block.getNumberOfKeys() <<" keys" << endl;
    return ( block.getNumberOfKeys() < minNumKeys );
}

/*
 * returns whether or not the block is the root of the tree
 * @return bool: true if block is root, false if not
 */
bool BTree::isRoot ( BTreeFile::BlockNumber numBlock ) const
{
    BTreeFile::BlockNumber numRoot = _file.getRoot();
    return ( numRoot == numBlock );
}

/*
 * removes a key value pair corresponding to the inputted key
 * from the binary tree
 * @param1 string: key of key value pair to remove
 * @return bool: true if found and removed, false if not found
*/
bool BTree::remove(string key)
{
    cout << endl;

    // will hold the number of the block where the key is found
    BTreeFile::BlockNumber numCurr = _file.getRoot();

    // will hold the path to the block containing the key
    std::stack<BTreeFile::BlockNumber> path;

    // sets numCurr equal to the block where key is found,
    // sets numParent equal to numCurr's parent,
    // and sets found to true or false dependent on if
    // the key is found in the tree
    bool found = find ( key, numCurr, path );

    // if the key is not found in the tree, return false
    if ( !found ) {
        cout << "Not found!" << endl;
        return false;
    } else {
        cout << "Found in: " << numCurr << endl;
        // set curr equal to the block with number numCurr
        BTreeBlock curr;
        _file.getBlock(numCurr, curr);

        // while curr is not a leaf, follow the tree down
        // to the first key in the leftmost subtree of the child
        // of curr just after the key, and then promote that key
        if (!curr.isLeaf()) {
            cout << "curr is not a leaf" << endl;
            // block that will become the leaf to promote from
            BTreeBlock leaf = curr;

            // position of child to follow - one after the key
            int position = leaf.getPosition(key) + 1;

            cout << "leaf.getPosition(key) + 1: " << position << endl;
            // number of the child just after the key in curr
            BTreeFile::BlockNumber  numLeaf = leaf.getChild(position);

            // set leaf equal to the block at numLeaf
            _file.getBlock( numLeaf, leaf );

            // follow the leftmost child of leaf until we get to a leaf
            while (!leaf.isLeaf()) {
                cout << "In while leaf is not a leaf" << endl;

                // number of the leftmost child of leaf
                numLeaf = leaf.getChild(0);

                // set leaf equal to the block at numLeaf
                _file.getBlock( numLeaf, leaf );
            }

            // set the key and value in curr that are to be removed
            // to the leftmost key and value of leaf
            position = curr.getPosition(key);
            curr.setKey(position, leaf.getKey(0));
            curr.setValue(position, leaf.getValue(0));

            // number of keys in the leaf from which we will promoted
            int numKeys = leaf.getNumberOfKeys();

            // slide the keys in leaf to the left
            for (int i = 0; i < numKeys - 1; i++) {
                cout << "Sliding keys in leaf" << endl;
                leaf.setKey(i, leaf.getKey(i+1));
                leaf.setValue(i, leaf.getValue(i+1));
            }

            // decrement the number of keys in leaf
            leaf.setNumberOfKeys(numKeys-1);

            // write the block where key was found to disk
            _file.putBlock(numCurr, curr);

            // set curr equal to the leaf from which we removed
            curr = leaf;

            // set numCurr equal to numLeaf
            numCurr = numLeaf;
        } else {
            // curr is a leaf
            cout << "In is leaf" << endl;

            // position of key to remove
            int position = curr.getPosition(key);

            // number of keys in the block containing the key
            int numKeys = curr.getNumberOfKeys();

            cout << "position before deletion: " << position << endl;

            // remove the key by sliding all of the keys over to the left
            // and decrementing the number of keys in curr
            for ( int i = position + 1; i < numKeys; i++ ) {
                cout << "curr.getkey(i): " << curr.getKey(i) << endl;
                cout << "curr.getkey(i-1): " << curr.getKey(i-1) << endl;
                curr.setKey(i-1, curr.getKey(i));
                curr.setValue(i-1, curr.getValue(i));
            }

            // decrement number of keys in curr
            curr.setNumberOfKeys( numKeys - 1 );

            // reset numKeys to current value of curr
            numKeys = curr.getNumberOfKeys();
        }

        cout << "num curr before needs keys checks: " << numCurr << endl;
        if (!needsKeys(numCurr, curr)) {
            cout << "curr does not need keys" << endl;

            _file.putBlock(numCurr, curr);
        } else if (isRoot(numCurr)) {
            // curr needs keys and is the root
            cout << "curr needs keys and is the root" << endl;
            _file.setRoot(0);
            _file.deallocateBlock(numCurr);
        } else {
            // curr needs keys and is not the root
            cout << "curr needs keys and is not the root" << endl;

            //temporary
            _file.putBlock(numCurr, curr);
        }
    }
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

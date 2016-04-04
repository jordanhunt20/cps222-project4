/*
 * BTreeFile.h - declaration of the nested class BTreeFile, used as part
 * of actually implementing a B Tree.  CPS222 project 4
 *
 * Copyright (c) 2001, 2002, 2003 - Russell C. Bjork
 * Trivial changes 2016 - Russ Tuck
 */

#ifndef BTREE_FILE_H
#define BTREE_FILE_H

#include <string>
#include <cstdio>
using namespace std;

// DEGREE: maximum number of children a node in the tree can have on disk.
#ifndef DEGREE
#define DEGREE 7
#endif

// Incomplete definition of class BTreeBlock is needed here because that class
// and this refer mutually to each other
class BTreeBlock;

/*
 * Each BTree is associated with an object of this class, which provides access
 * to the disk file in which the BTree is stored.
 */

class BTreeFile
{
  public:

    /* Individual blocks in the file are referred to by block number
     * (1 .. size of file)
     */
    typedef unsigned BlockNumber;

    /* Constructor - parameter has the same meaning as the BTree constructor.
     * This actually creates the access path to the file.
     */
    BTreeFile(string name);

    // Accessor for name
    string getName() const;

    // Accessor for block number of root of the tree - 0 means
    // the tree is empty (has no root)
    BlockNumber getRoot() const;

    // Mutator to change the root block of the tree
    void setRoot(BlockNumber root);

    /* Access information stored in a particular block of the file.
     * number must be in the range 1 .. size of the disk file.
     * block will contain an in-memory copy of the current contents
     * of this block in the file.  Returns true if the operation
     * succeeds, false if it fails (e.g. due to an invalid block
     * number or some IO system error.)
     */
    bool getBlock(BlockNumber number, BTreeBlock & block);

    /* Update information stored in a particular block of the file.
     * number must be in the range 1 .. size of the disk file.
     * The information stored in memory in block will be copied to the
     * specified block of the file.	  Returns true if the operation
     * succeeds, false if it fails (e.g. due to an invalid block
     * number, or some IO system error.)
     */
    bool putBlock(BlockNumber number, const BTreeBlock & block);

    // Allocate a block to be used for a new node in the tree
    BlockNumber allocateBlock();

    // Deallocate a block that was used for a node in the tree,
    // but is not needed anymore
    void deallocateBlock(BlockNumber number);

    // Print out header information to cout
    void printHeaderInfo() const;

    /* Print information in a specific block to cout. If recursive is true,
     * level is printed out, and controls the indentation of the block.
     * Further, if recursive is true, the child blocks are printed out as
     * well.  If recursive is false, only the specified block is printed,
     * and level is ignored.  (This method is provided to support the
     * print() methods in BTree.h that in term support testing/debugging/)
     */
    void printBlock(BlockNumber number, bool recursive, unsigned level);

    // Destructor - print out statistics
    ~BTreeFile();

  private:

    string _name;		// File name
    unsigned _degree;		// Degree of tree
    BlockNumber _root;		// Number of root block
    BlockNumber _lastUsed;	// Number of the last block currently in use
    BlockNumber _free;		// Pointer to a linked list of recycled blocks
    FILE * _stream;		// The stdio stream actually used

    // The following private class represents the way data is
    // physically stored on the disk.
    class PhysicalBlock;

    /* Actually physically transfer data to/from disk.
     * Return true if successful.  (False typically indicates use
     * of an invalid block number, or some sort of IO system
     * failure.)
     */
    bool getBlock(BlockNumber number, PhysicalBlock & buffer);
    bool putBlock(BlockNumber number, const PhysicalBlock & buffer);

    // Update copy of header on disk
    void updateHeader();

    // Counters to keep track of statistics for testing
    unsigned _totalGets, _totalPuts,
        _headerGets, _headerPuts,
        _freeGets, _freePuts;
};

#endif

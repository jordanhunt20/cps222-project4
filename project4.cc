/*
 * project4.cc
 *
 * Main program for CS212 project 4 - test driver to create and test
 * a B-Tree
 *
 * Copyright (c) 2001, 2003 - Russell C. Bjork
 * Trivial changes 2016 by Russ Tuck
 */

#include <iostream>
#include <string>
using std::cin;
using std::cout;

#include "BTree.h"

int main(int argc, char * argv[])
{
    string filename;

    cout << "Name of file containing the B-Tree: ";
    cin >> filename;

    BTree theTree(filename);

    // Repeatedly read commands from standard input and perform them.
    // The following commands (or their lower-case equivalents) are allowed.
    // Each command stands by itself on a line.	 Whitespace may preceed the
    // command and may separate the command and key.  The key must be a
    // single token not containing any whitespace.	In the case of insert,
    // everything on the line after the first space is taken as the value,
    // even if it contains spaces.
    //
    // I key value - insert a key/value pair
    // L key - lookup a key; print corresponding value or report none
    // R key - remove a key/value pair
    // P print entire tree to cout
    // D dump specific block(s) to cout
    // Q quit

    char command;
    string key, value;

    do {
        cin >> command;
        if (cin.eof()) command = 'Q';

        switch(command) {

          case 'I': case 'i':
            cin >> key;
            cin.get();		// Skip one space
            getline(cin, value);
            cout << "Inserting	" << key << ' ' << value << endl;
            theTree.insert(key, value);
            break;

          case 'L': case 'l':
            cin >> key;
            while (cin.get() != '\n') ;
            cout << "Looking up " << key;
            if (theTree.lookup(key, value))
                cout << " - Found: " << value << endl;
            else
                cout << " - Not found" << endl;
            break;

          case 'R': case 'r':
            cin >> key;
            while (cin.get() != '\n') ;
            cout << "Removing	" << key;
            if (theTree.remove(key))
                cout << " - Removed" << endl;
            else
                cout << " - Remove failed" << endl;
            break;

          case 'P': case 'p':
            while (cin.get() != '\n') ;
            theTree.print();
            cout << endl;
            break;

          case 'D': case 'd':
            {
                int start, finish, c;
                cin >> start;
                while ((c = cin.get()) == ' ') ;
                if (c == '\n')
                    finish = start;
                else
                {
                    cin.unget();
                    cin >> finish;
                    while (cin.get() != '\n') ;
                }
                for (int i = start; i <= finish; i ++)
                    theTree.print(i);
                break;
            }

          case 'Q': case 'q':
            cout << "Finished" << endl;
            break;

          default:
            while (cin.get() != '\n')
                ;
            cout <<	 "Valid commands are:" << endl <<
                "I(nsert) key value; L(ookup) key; R(emove) key; P(rint); D(ump) start [finish] Q(uit)"
                 << endl;
            break;
        }

        cout << endl;

    } while (command != 'Q' && command != 'q');
}

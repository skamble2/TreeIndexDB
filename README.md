# TreeIndexDB

**TreeIndexDB** is an advanced database management system designed for efficient B+ Tree operations and comprehensive index management. This system is optimized for high performance, providing robust functionalities for creating, inserting, deleting, and finding records within a B+ Tree structure. It also offers powerful scanning capabilities, allowing users to traverse the tree in sorted order. TreeIndexDB integrates a full-featured index manager, facilitating the creation and maintenance of B+ Tree indexes, making it an ideal solution for applications that require fast, reliable access to large datasets.

## Table of Contents

- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [Functions Overview](#functions-overview)
  - [B+ Tree Operations](#b-tree-operations)
  - [Index Manager Functions](#index-manager-functions)
  - [Record Management](#record-management)
  - [Buffer Pool Management](#buffer-pool-management)
  - [Custom B+ Tree Functions](#custom-b-tree-functions)
- [Contributors](#contributors)

## Features

- **Efficient B+ Tree Operations**: Includes creation, insertion, deletion, and searching within B+ Trees.
- **Custom Node Management**: Features custom functions for node splitting, merging, and redistribution.
- **Comprehensive Index Management**: Initialize and manage B+ Tree indexes with ease.
- **Record Management**: Efficient handling of records with functionalities to insert, delete, and update records.
- **Buffer Pool Management**: Advanced buffer management with support for FIFO and LRU strategies.
- **Scanning Capabilities**: Traverse and scan tree entries in sorted key order.

## Installation

To install and set up TreeIndexDB, follow these steps:

1. Clone the repository:
   ```bash
   git clone https://github.com/TreeIndexDB.git
   cd TreeIndexDB
   ```

2. Compile the code using `make`:
   ```bash
   make
   ```

3. Run the program:
   ```bash
   make run
   ```

4. Clean up binary files and objects:
   ```bash
   make clean
   ```

## Usage

To use TreeIndexDB, follow the instructions below:

- Open a terminal and navigate to the project directory.
- Use the `make` command to compile the code.
- Use the `make run` command to execute the program.
- Use the `make clean` command to remove any compiled binaries and object files.

## Functions Overview

### B+ Tree Operations

- **createBtree(...)**: Initializes and creates a new B+ Tree.
- **openBtree(...)**: Opens an existing B+ Tree stored in a file.
- **closeBtree(...)**: Closes the B+ Tree, marking pages dirty and writing back to disk.
- **deleteBtree(...)**: Deletes the specified B+ Tree file.

### Index Manager Functions

- **initIndexManager(...)**: Initializes the index manager and its resources.
- **shutdownIndexManager(...)**: Shuts down the index manager and frees resources.

### Record Management

- **insertRecord(...)**: Inserts a new record into the B+ Tree.
- **deleteRecord(...)**: Deletes a record from the B+ Tree.
- **updateRecord(...)**: Updates an existing record in the B+ Tree.
- **getRecord(...)**: Retrieves a record from the B+ Tree.

### Buffer Pool Management

- **initBufferPool(...)**: Initializes a buffer pool with a specified number of pages.
- **shutdownBufferPool(...)**: Closes the buffer pool and frees up resources.
- **forceFlushPool(...)**: Writes all dirty pages from the buffer pool to disk.
- **unpinPage(...)**: Unpins a page, allowing it to be flushed from memory.

### Custom B+ Tree Functions

- **findLeaf(...)**: Finds the leaf node containing a specified key.
- **insertIntoLeaf(...)**: Inserts a new record pointer and key into a leaf.
- **insertIntoNode(...)**: Inserts a key and pointer into a non-leaf node.
- **adjustRoot(...)**: Adjusts the root after deletion to maintain B+ Tree properties.
- **deleteEntry(...)**: Deletes an entry from the B+ Tree and adjusts nodes accordingly.
- **findKey(...)**: Searches for a specified key in the B+ Tree.
- **openTreeScan(...)**: Initializes a scan to traverse entries in sorted order.
- **nextEntry(...)**: Retrieves the next entry in the scan.
- **closeTreeScan(...)**: Closes the scan and frees resources.

## Contributors

- **Karan Batra** (A20518491) - [kbatra3@hawk.iit.edu](mailto:kbatra3@hawk.iit.edu)
  - Custom B+ Tree Functions, Buffer Pool Management
- **Soham Kamble** (A20517098) - [skamble2@hawk.iit.edu](mailto:skamble2@hawk.iit.edu)
  - Record Management, B+ Tree Operations
- **Divyansh Prakhar Soni** (A20517331) - [dsoni2@hawk.iit.edu](mailto:dsoni2@hawk.iit.edu)
  - Index Manager, Schema Management

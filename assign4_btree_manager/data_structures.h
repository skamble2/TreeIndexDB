#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include "dt.h"
#include "dberror.h"

// // Assuming that we need 8KB of memory per page and we want to allocate a total of 10MB for each pool, we will need 1250 pages. Since the page allocation scheme uses a hash table, we want to choose a prime number close to 1250 to reduce collisions. 1259 is the closest prime number to 1250.
#define HASH_LEN 1259

// linked list Node
typedef struct Node {
  void *data; // a pointer to any type of data that will be stored in this node
  struct Node *next; // a pointer to the next node in the linked list
  struct Node *previous; // a pointer to the previous node in the linked list
} Node;

// A key-value combination for a hash map
typedef struct HM_Comb {
  int key;
  void *val;
} HM_Comb;

// Hashmap data structure with an array of linked lists to handle collisions
typedef struct HM {
  Node *tbl[HASH_LEN]; 
} HM;

// dynamic array with automatic sorting
// The smartArray is an array data structure that automatically sorts its elements as new elements are inserted.
// The elements are stored as an array of integers and are sorted in ascending order. The size field indicates the allocated size of the array,
// and the fill field indicates the current number of elements in the array. The elems field is a pointer to the array of integers.
typedef struct smartArray {
  int size; // allocated size of the array
  int fill;  // current number of elements in the array
  int *elems; // array of integers
} smartArray;


// initialize a new hashmap
HM *hmInit();
// calculate hash value of given key
int hash(int key);
// insert a new key-value pair into the hashmap
int hmInsert(HM *hm, int key, void *val);
// get the value associated with the given key
void *hmGet(HM *hm, int key);
// delete the key-value pair with the given key from the hashmap
int hmDelete(HM *hm, int key);
// free memory used by the hashmap
void hmDestroy(HM *hm);

// smartArray management functions
smartArray *saInit(int size); // Initialize a new smartArray with a given size
int saBinarySearch(smartArray *arr, int elem, int *fitOn); // Perform a binary search on the
void saDestroy(smartArray *arr); // Destroy a smartArray and free up the memory allocated to it
int saInsertAt(smartArray *arr, int elem, int index);  // Insert a new element at a given index in the smartArray
int saInsert(smartArray *arr, int elem); // Insert a new element in the sorted smartArray
void saDeleteAt(smartArray *arr, int index, int count); // Delete a given number of elements starting from a given index in the smartArray
int saDeleteOne(smartArray *arr, int elem); // Delete the first occurrence of a given element in the smartArray
int saDeleteAll(smartArray *arr, int elem); // Delete all occurrences of a given element in the smartArray
void saEmpty(smartArray *arr); // Remove all elements from the smartArray
void saPrint(smartArray *arr); // Print the elements in the smartArray

#endif

#include "data_structures.h"
#include <stdlib.h>
#include <stdio.h>

/**************************************************************************************
 * Function Name: hmInit
 *
 * Description:
 *		used to initialize a new hash table of type HM. The function allocates memory for the hash table and sets all of its linked list nodes to NULL.
 *
 * Parameters:
 *		
 *
 * Return:
 *		HM *: pointer to the newly initialized hash table. If the allocation of memory for the hash table fails, the function returns NULL. 
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

HM *hmInit()
{
  HM *hm = NULL;
  hm = malloc(sizeof(HM));
  int i;
  i = 0;
  do
  {
    hm->tbl[i] = NULL;
    i++;
  } while (i < HASH_LEN);

  return hm;
}

/**************************************************************************************
 * Function Name: hash
 *
 * Description:
 *		used to generate an index into a hash table based on an integer key. The function uses the XOR and multiplication operations to generate a hash value from the key.
 *
 * Parameters:
 *		int key: integer representing the key to be hashed.
 *
 * Return:
 *		int: an integer representing the calculated index in the hash table where the key-value pair will be stored. The returned index is determined by taking the modulus of the calculated hash value with the constant HASH_LEN, which represents the number of buckets in the hash table.
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

int hash(int key)
{
  unsigned int hash_value = 0;
  unsigned int i = 0;

  
  i = 0;
  do
  {
    hash_value ^= ((key >> (i * 8)) & 0xFF);
    hash_value *= 16777619;
    i++;
  } while (i < sizeof(key));

  
  return hash_value % HASH_LEN;
}

/**************************************************************************************
 * Function Name: getComb
 *
 * Description:
 *		used to retrieve an HM_Comb object from the hash table "hm" based on its "key". The function calculates the index of the hash table where the key-value pair is expected to be stored, then loops through the linked list at that index until it finds a node with a matching key or reaches the end of the list. If a node with a matching key is found, the HM_Comb object stored in that node is returned. If the key is not found, the function returns NULL.
 *
 * Parameters:
 *		HM *hm:  a pointer to an HM hash table.
 *    int key: an integer representing the key for the HM_Comb object to retrieve.
 *
 * Return:
 *		HM_Comb: a pointer to the HM_Comb object with the matching key, or NULL if the key is not found in the hash table.
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/
HM_Comb *getComb(HM *hm, int key)
{
  int indx = hash(key);
  Node *head = hm->tbl[indx];
  HM_Comb *comb = NULL;

  
  do
  {
    comb = (HM_Comb *)(head->data);

    
    if (key == comb->key)
    {
      return comb;
    }

    
    head = head->next;
  } while (head);

  
  return NULL;
}

/**************************************************************************************
 * Function Name: hmInsert
 *
 * Description:
 *		used to insert a new key-value pair into the hash table "hm". The function first checks if the key already exists in the hash table by calling the "getComb" function. If the key is found, the function updates the value associated with the key. If the key is not found, the function creates a new HM_Comb object to store the key-value pair and inserts it into the hash table using a linked list.
 *
 * Parameters:
 *		HM *hm: a pointer to an HM hash table.
 *    int key: an integer representing the key to be inserted.
 *    void *val: a pointer to the value to be associated with the key.
 * 
 * Return:
 *		int: an integer representing the success of the insert operation. The function always returns 1, indicating that the operation was successful.
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

int hmInsert(HM *hm, int key, void *val)
{
  HM_Comb *comb = getComb(hm, key);


  comb != NULL ? (comb->val = val, 0) : 1;

  
  comb = malloc(sizeof(HM_Comb));
  comb->key = key;
  comb->val = val;
  Node *newNode = malloc(sizeof(Node));
  newNode->data = comb;
  newNode->next = NULL;
  newNode->previous = NULL;

  int indx = hash(key);
  Node *head = hm->tbl[indx];

  
  newNode->next = (head != NULL) ? head : NULL;
  newNode->previous = (head != NULL) ? NULL : head;
  if (head)
    head->previous = newNode;
  hm->tbl[indx] = newNode;

  
  hm->tbl[indx] = newNode;

  return 1;
}

/**************************************************************************************
 * Function Name: hmGet
 *
 * Description:
 *		get value by key from hashmap
 *
 * Parameters:
 *		HM *hm: a pointer to an HM hash table.
 *    int key: an integer representing the key to retrieve the value for.
 *
 * Return:
 *		void
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

void *hmGet(HM *hm, int key)
{
  HM_Comb *comb = getComb(hm, key);
  return (comb != NULL) ? comb->val : NULL;
}

/**************************************************************************************
 * Function Name: hmDelete
 *
 * Description:
 *		delete a key, value pair from the hashmap
 *
 * Parameters:
 *		HM *hm: a pointer to an HM hash table.
 *    int key: an integer representing the key to delete the value for.
 *
 * Return:
 *		int: The function returns 1 if the key was found and the corresponding HM_Comb object was deleted, or 0 if the key was not found in the hash table.
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

int hmDelete(HM *hm, int key)
{
  int indx = hash(key);
  Node *head = hm->tbl[indx];
  HM_Comb *comb = NULL;
  do
  {
    comb = (HM_Comb *)(head->data);
    if (comb->key == key)
    {
      hm->tbl[indx] = (head->previous == NULL) ? head->next : hm->tbl[indx];
      (head->previous != NULL) ? (head->previous->next = head->next) : (void)0;
      (head->next != NULL) ? (head->next->previous = head->previous) : (void)0;

      free(comb);
      free(head);
      return 1; 
    }
    head = head->next;
  } while (head != NULL);

  return 0; 
}

/**************************************************************************************
 * Function Name: hmDestroy
 *
 * Description:
 *		delete all nodes of hashmap and release resources
 *
 * Parameters:
 *		HM *hm: a pointer to the hash map that needs to be destroyed.
 *
 * Return:
 *		void
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

void hmDestroy(HM *hm)
{
  Node *deletable = NULL;
  int i;
  i = 0;
  do
  {
    Node *head = hm->tbl[i];
    Node *deletable;
    do
    {
      deletable = head;
      head = head->next;
      free(deletable->data);
      free(deletable);
    } while (head != NULL);
    i++;
  } while (i < HASH_LEN);

  free(hm);
}

/**************************************************************************************
 * Function Name: saPrint
 *
 * Description:
 *		This function prints the elements of a given smartArray to the console.
 *
 * Parameters:
 *		smartArray *arr: a pointer to the smartArray to be printed
 *
 * Return:
 *		void
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

void saPrint(smartArray *arr)
{
  return;

  int i = 0;
  do
  {
    printf("%d ", arr->elems[i]);
    i++;
  } while (i < arr->fill);
}

/**************************************************************************************
 * Function Name: saBinarySearch
 *
 * Description:
 *		performs binary search on the smartArray arr to find the index of the element elem. It also sets the value of fitOn to the index where the elem could be inserted to maintain the sorted order of the array.
 *
 * Parameters:
 *		smartArray *arr: A pointer to a smartArray structure.
 *    int elem: An integer value that needs to be searched in the smartArray.
 *    int *fitOn: A pointer to an integer variable that will be set to the index where the elem could be inserted to maintain the sorted order of the array.
 *
 * Return:
 *		int: If the elem is found in the arr, the function returns its index.
 *    If the elem is not found in the arr, the function returns -1.
*     Note: the index is a zero-based index.
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

int saBinarySearch(smartArray *arr, int elem, int *fitOn)
{
  int first = 0;
  int last = arr->fill - 1;
  if (last < 0)
  { 
    (*fitOn) = first;
    return -1;
  }
  int pos;
  while (true)
  {
    pos = (first + last) / 2;
    if (elem == arr->elems[pos])
    {
      while (pos && elem == arr->elems[pos - 1]) pos--;

      (*fitOn) = pos;
      return pos;
    }
    if (first >= last)
    {
      if (elem > arr->elems[first])
      {
        first++;
      }
      (*fitOn) = first; 
      return -1;        
    }
    (elem < arr->elems[pos]) ? (last = pos - 1) : (first = pos + 1);

  }
}

/**************************************************************************************
 * Function Name: saInit
 *
 * Description:
 *		 Initializes a new smartArray and returns a pointer to it.
 *
 * Parameters:
 *		int size: the size of the smartArray to be initialized
 *
 * Return:
 *		smartArray *: a pointer to the newly initialized smartArray
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

smartArray *saInit(int size)
{
  smartArray *arr = (smartArray *)calloc(1, sizeof(smartArray));
  arr->elems = newIntArr(size);
  arr->size = size;
  arr->fill = 0;
  return arr;
}

/**************************************************************************************
 * Function Name: saDestroy
 *
 * Description:
 *		a function that frees the memory allocated for a given smartArray struct and its underlying array of elements.
 *
 * Parameters:
 *		smartArray *arr: a pointer to the smartArray struct that needs to be destroyed.
 *
 * Return:
 *		void
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

void saDestroy(smartArray *arr)
{
  if (arr)
  {
    free(arr->elems);
    free(arr);
  }
}

/**************************************************************************************
 * Function Name: saInsertAt
 *
 * Description:
 *		a function that inserts a new element into a smartArray at a specified index.
 *
 * Parameters:
 *		smartArray *arr: a pointer to a smartArray struct representing the array to insert the element into
 *	  int elem: an integer representing the element to insert
 *    int index: an integer representing the index at which to insert the new element
 *
 * Return:
 *		If the insertion is successful, the function returns the index at which the new element was inserted.
 *    If the insertion is unsuccessful (e.g. the array is already full or the index is out of range), the function returns -1.
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

int saInsertAt(smartArray *arr, int elem, int index)
{
  if ((index <= arr->fill) && (arr->size > arr->fill)

)
  {
    if (index != arr->fill)
    { 
      int i = arr->fill;
      do
      {
        arr->elems[i] = arr->elems[i - 1];
        i--;
      } while (i > index);
    }
    arr->elems[index] = elem;
    arr->fill++;
    return index;
  }
  return -1; 
}

/**************************************************************************************
 * Function Name: saInsert
 *
 * Description:
 *		inserts an element into a smartArray while keeping it sorted in ascending order. It first searches for the index where the element should be inserted using binary search, then inserts the element at the calculated index using saInsertAt function. Finally, it calls saPrint function to print the elements of the updated smartArray and returns the index where the element was inserted.
 *
 * Parameters:
 *		smartArray *arr: A pointer to the smartArray to be updated with the new element.
 *    int elem:  The integer element to be inserted into the smartArray.
 *
 * Return:
 *		int:  which is the index where the element was inserted into the smartArray, or -1 if the insertion was not successful (i.e., the smartArray is already full).
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

int saInsert(smartArray *arr, int elem)
{
  int fitOn = -1; 
  if (arr->fill < arr->size)
  {
    int index = saBinarySearch(arr, elem, &fitOn);
    fitOn = saInsertAt(arr, elem, fitOn);
  }
  saPrint(arr);
  return fitOn;
}

/**************************************************************************************
 * Function Name: saDeleteAt
 *
 * Description:
 *		function removes count number of elements from the smartArray starting from the specified index. It shifts the elements to the left to fill the gap left by the removed elements and updates the fill attribute of the smartArray accordingly.
 *
 * Parameters:
 *		smartArray *arr: A pointer to the smartArray structure.
 *    int index: An integer representing the index of the first element to be removed.
 *    int count: An integer representing the number of elements to be removed.
 *
 * Return:
 *		void
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

void saDeleteAt(smartArray *arr, int index, int count)
{
  arr->fill -= count;
  int i = index;
  do
  {
    arr->elems[i] = arr->elems[i + count];
    i++;
  } while (i < arr->fill);
}

/**************************************************************************************
 * Function Name: saDeleteOne
 *
 * Description:
 *		Deletes one occurrence of a given element from the smart array.
 *
 * Parameters:
 *		smartArray *arr: Pointer to a smartArray struct representing the smart array.
 *    int elem: The element to be deleted from the smart array.
 *
 * Return:
 *		Returns the index of the deleted element if it was found and deleted.
 *    Returns -1 if the element was not found in the smart array.
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

int saDeleteOne(smartArray *arr, int elem)
{
  int _unused;
  int index = saBinarySearch(arr, elem, &_unused);
  (index >= 0) ? saDeleteAt(arr, index, 1) : 0;

  saPrint(arr);
  return index;
}

/**************************************************************************************
 * Function Name: saDeleteAll
 *
 * Description:
 *		This function takes a pointer to a smartArray and an integer elem, and deletes all occurrences of elem from the array.
 *
 * Parameters:
 *		smartArray *arr:  A pointer to a smartArray.
 *    int elem: An integer representing the element to be deleted.
 *
 * Return:
 *		The index of the first occurrence of elem before deletion or -1 if elem is not found.
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

int saDeleteAll(smartArray *arr, int elem)
{
  int _unused;
  int index = saBinarySearch(arr, elem, &_unused);
  if (index > 0)
  {
    int count = 1;
    int tempCount = count;
    do {
  tempCount += (elem == arr->elems[index + tempCount]) ? 1 : 0;
} while (index + tempCount < arr->fill);

    count = tempCount;

    saDeleteAt(arr, index, count);
  }
  saPrint(arr);
  return index;
}

/**************************************************************************************
 * Function Name: saEmpty
 *
 * Description:
 *		empties the smartArray by setting the fill parameter of the array to 0.
 *
 * Parameters:
 *		smartArray *arr: The function takes a pointer to a smartArray arr as its argument.
 *
 * Return:
 *		void
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

void saEmpty(smartArray *arr)
{
  arr->fill = 0;
}

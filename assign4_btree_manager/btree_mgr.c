// Assignment 4 Btree Manager
// Group 20
// Soham Kamble (A20517098) <skamble2@hawk.iit.edu>
// Karan Batra (A20518491)  <kbatra3@hawk.iit.edu>
// Leily Nourbaksh (A20516826) <snoorbakhsh@hawk.iit.edu>

#include "data_structures.h"
#include "btree_mgr.h"
#include "storage_mgr.h"
#include "buffer_mgr.h"
#include <stdarg.h>
#include <unistd.h>
#include <math.h>
#include "string.h"

/**************************************************************************************
 * Function Name: freeit
 *
 * Description:
 *		This function frees up memory that has been dynamically allocated by a program using the "malloc", "calloc", or "realloc" functions. It takes a variable number of arguments and iterates over them to free each pointer that is not NULL.
 *
 * Parameters:
 *		int num: specifies the number of pointers to free.
 *    ...: variadic argument that can take any number of void pointers that need to be freed.
 *
 * Return:
 *		void
 *
 * Author:
 *		Karan Batra <kbatra3@hawk.iit.edu> (A20518491)
 **************************************************************************************/
void freeit(int num, ...)
{
  va_list valist;
  void *ptr = NULL;

  va_start(valist, num);
  int i = 0;
  do
  {
    ptr = va_arg(valist, void *);
    if (ptr != NULL)
    {
      free(ptr);
    }
    i++;
  } while (i < num);
  va_end(valist);
}

/**************************************************************************************
 * Function Name: createBTNode
 *
 * Description: This function creates a new node for a B-Tree, initializes its properties, and returns a pointer to the newly created node.
 *
 *
 * Parameters:
 *		int size: specifies the maximum number of keys that the node can hold.
 *    int isLeaf: indicates whether the node is a leaf node or not. If it is a leaf node, its value is 1; otherwise, its value is 0.
 *    int pageNum: specifies the page number of the node in a persistent storage system, if any.
 *
 * Return:
 *		BT_Node: pointer to a BT_Node struct, which represents a node in a B-Tree. This pointer can be used to access and manipulate the properties of the node.
 *
 * Author:
 *		Karan Batra <kbatra3@hawk.iit.edu> (A20518491)
 **************************************************************************************/
BT_Node *createBTNode(int size, int isLeaf, int pageNum)
{
  BT_Node *node = (BT_Node *)malloc(sizeof(BT_Node));

  (node == NULL) ? NULL : node;

  node->isLeaf = isLeaf;
  node->size = size;
  node->pageNum = pageNum;
  node->right = NULL;
  node->left = NULL;
  node->parent = NULL;
  node->vals = saInit(size);

  isLeaf ? (node->leafRIDPages = saInit(size), node->leafRIDSlots = saInit(size)) : (node->childrenPages = saInit(size + 1), node->children = newArray(BT_Node *, size + 1));

  return node;
}

/**************************************************************************************
 * Function Name: destroyBTNode
 *
 * Description:
 *		This function frees up memory that has been dynamically allocated for a given BT_Node struct, and its associated arrays.
 *
 * Parameters:
 *		BT_Node *node: pointer to the BT_Node struct that needs to be destroyed.
 *
 * Return:
 *		void
 *
 * Author:
 *		Karan Batra <kbatra3@hawk.iit.edu> (A20518491)
 **************************************************************************************/
void destroyBTNode(BT_Node *node)
{
  if (!node)
  {
    return;
  }

  saDestroy(node->vals);

  (node->isLeaf) ? (saDestroy(node->leafRIDPages), saDestroy(node->leafRIDSlots)) : (saDestroy(node->childrenPages), (node->children != NULL) ? free(node->children) : 0);

  free(node);
}

/**************************************************************************************
 * Function Name: printNodeHelper
 *
 * Description:
 *		This function is a helper function used to print the contents of a BT_Node struct to a string buffer. It takes in a pointer to the node to be printed and a pointer to a character buffer that will hold the resulting string. The function fills the buffer with a string representation of the node's contents.
 *
 * Parameters:
 *		BT_Node *node: pointer to the BT_Node struct that needs to be printed.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

RC printNodeHelper(BT_Node *node, char *result)
{
  (node == NULL) ? (sprintf(result, "NULL Node !!\n"), RC_GENERAL_ERROR) : 0;

  sprintf(result, "(%d)[", node->pageNum);

  int i;

  if (node->isLeaf)
  {
    i = 0;
    do
    {
      sprintf(result + strlen(result), "%d.%d,%d", node->leafRIDPages->elems[i], node->leafRIDSlots->elems[i], node->vals->elems[i]);
      (i < node->vals->fill - 1) ? sprintf(result + strlen(result), ",") : 0;

      i++;
    } while (i < node->vals->fill);
  }
  else
  {
    i = 0;
    do
    {
      sprintf(result + strlen(result), "%d,%d", node->childrenPages->elems[i], node->vals->elems[i]);
      i < node->vals->fill - 1 ? sprintf(result + strlen(result), ",") : 0;

      i++;
    } while (i < node->vals->fill);

    sprintf(result + strlen(result), "%d", node->childrenPages->elems[i]);
  }

  sprintf(result + strlen(result), "]\n");

  return RC_OK;
}

/**************************************************************************************
 * Function Name: readNode
 *
 * Description:
 *		This function reads a node from a page with a given page number in a B+ Tree and creates a BT_Node struct to hold the node's data.
 *
 * Parameters:
 *		BT_Node **node: A pointer to a pointer to a BT_Node struct, which will hold the data of the read node.
 *	  BTreeHandle *tree: A pointer to a BTreeHandle struct, which holds information about the B+ Tree.
 *    int pageNum: An integer representing the page number of the node to be read.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

RC readNode(BT_Node **node, BTreeHandle *tree, int pageNum)
{
  RC err;
  BM_PageHandle *page = new (BM_PageHandle);

  return (RC_OK != (err = pinPage(tree->mgmtData, page, pageNum))) ? err : (void)free(page), err;

  char *ptr = page->data;
  int isLeaf, fill;
  memcpy(&isLeaf, ptr, SIZE_INT);
  memcpy(&fill, ptr + SIZE_INT, SIZE_INT);
  ptr += BYTES_BT_HEADER_LEN;

  BT_Node *new_node = createBTNode(tree->size, isLeaf, pageNum);
  int value, i;

  if (!isLeaf)
  {
    int child_page;
    i = 0;
    do
    {
      memcpy(&child_page, ptr, SIZE_INT);
      ptr += SIZE_INT;

      memcpy(&value, ptr, SIZE_INT);
      ptr += SIZE_INT;

      saInsertAt(new_node->vals, value, i);
      saInsertAt(new_node->childrenPages, child_page, i);
      i++;
    } while (i < fill);

    memcpy(&child_page, ptr, SIZE_INT);
    saInsertAt(new_node->childrenPages, child_page, i);
  }
  else
  {
    int ridPage, ridSlot;
    i = 0;
    do
    {
      memcpy(&ridPage, ptr, SIZE_INT);
      ptr += SIZE_INT;

      memcpy(&ridSlot, ptr, SIZE_INT);
      ptr += SIZE_INT;

      memcpy(&value, ptr, SIZE_INT);
      ptr += SIZE_INT;

      saInsertAt(new_node->vals, value, i);
      saInsertAt(new_node->leafRIDPages, ridPage, i);
      saInsertAt(new_node->leafRIDSlots, ridSlot, i);
      i++;
    } while (i < fill);
  }

  err = unpinPage(tree->mgmtData, page);
  free(page);

  *node = new_node;
  return err;
}

/**************************************************************************************
 * Function Name: writeNode
 *
 * Description:
 *		This function writes the contents of a BT_Node structure into a page in the buffer pool managed by the BTreeHandle structure.
 *
 * Parameters:
 *    		BT_Node *node: a pointer to the node that needs to be written to the page.
 *        BTreeHandle *tree: a pointer to the BTreeHandle structure that manages the buffer pool.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

RC writeNode(BT_Node *node, BTreeHandle *tree)
{
  RC err;
  BM_PageHandle *page = new (BM_PageHandle);

  (RC_OK != (err = pinPage(tree->mgmtData, page, node->pageNum))) ? (free(page), err) : 0;

  char *ptr = page->data;
  memcpy(ptr, &node->isLeaf, SIZE_INT);
  ptr += SIZE_INT;
  memcpy(ptr, &node->vals->fill, SIZE_INT);
  ptr = page->data + BYTES_BT_HEADER_LEN;
  int i;
  i = 0;

  err = markDirty(tree->mgmtData, page);

  err = unpinPage(tree->mgmtData, page);
  (err != RC_OK) ? (free(page), err) : RC_OK;
  do
  {
    int *pages = node->isLeaf ? node->leafRIDPages->elems : node->childrenPages->elems;
    int *vals = node->vals->elems;
    memcpy(ptr, &pages[i], SIZE_INT);
    ptr += SIZE_INT;
    memcpy(ptr, &vals[i], SIZE_INT);
    ptr += SIZE_INT;
    i++;
  } while (i < node->vals->fill);

  (!node->isLeaf) ? memcpy(ptr, &node->childrenPages->elems[i], SIZE_INT) : 0;

  err = markDirty(tree->mgmtData, page);
  err = unpinPage(tree->mgmtData, page);
  forceFlushPool(tree->mgmtData);
  free(page);
  return err;
}

/**************************************************************************************
 * Function Name: printNode
 *
 * Description:
 *		This function is used to print the details of a B+ tree node including whether it is a leaf node or not, its size, its fill, and its page number. It also prints the values of the node along with the RID (Record ID) information.
 *
 * Parameters:
 *		BT_Node *node: pointer to a BT_Node struct representing the node to be printed.
 *
 * Return:
 *		void
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/
void printNode(BT_Node *node)
{
  if (node == NULL)
  {
    printf("NULL Node");
    return;
  }
  printf("Node details==");
  printf("Is Leaf : %d Size: %d fill: %d pageNum: %d NodeData:", node->isLeaf, node->size, node->vals->fill, node->pageNum);
  int i;
  if (node->isLeaf)
  {
    i = 0;
    do
    {
      printf("%d.%d , %d", node->leafRIDPages->elems[i], node->leafRIDSlots->elems[i], node->vals->elems[i]);
      printf("%s", i != node->vals->fill - 1 ? "," : "");

      i++;
    } while (i < node->vals->fill);
  }
  else
  {
    i = 0;
    do
    {
      printf("%d ", node->childrenPages->elems[i]);
      printf("%d", node->vals->elems[i]);
      i++;
    } while (i < node->vals->fill);

    printf("%d", node->childrenPages->elems[i]);
  }
}

/**************************************************************************************
 * Function Name: loadBtreeNodes
 *
 * Description:
 *		This function recursively loads all the nodes in a B-tree into memory, starting from the root node.
 *
 * Parameters:
 *		BTreeHandle *tree: A pointer to a BTreeHandle struct representing the B-tree being loaded.
 *    BT_Node *root: A pointer to a BT_Node struct representing the root node of the B-tree.
 *    BT_Node **leftOnLevel: An array of BT_Node pointers representing the leftmost node on each level of the B-tree that has been loaded into memory so far.
 *    int level: An integer representing the level of the B-tree that is currently being loaded.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Karan Batra <kbatra3@hawk.iit.edu> (A20518491)
 **************************************************************************************/
RC loadBtreeNodes(BTreeHandle *tree, BT_Node *root, BT_Node **leftOnLevel, int level)
{
  BT_Node *left = leftOnLevel[level];
  RC err;
  int i;
  if (!root->isLeaf)
  {
    do
    {
      if ((err = readNode(&root->children[i], tree, root->childrenPages->elems[i])))
      {
        return err;
      }
      (left != NULL) ? (left->right = root->children[i]) : 0;

      root->children[i]->left = left;
      left = root->children[i];
      root->children[i]->parent = root;
      leftOnLevel[level] = left;

    } while (--i >= 0 && (err = loadBtreeNodes(tree, root->children[i], leftOnLevel, level + 1)) == RC_OK);

    return err;
  }
  return RC_OK;
}

/**************************************************************************************
 * Function Name: findNodeByKey
 *
 * Description:
 *		This function is used to find a node in a B-tree that contains a specific key. It starts from the root node and traverses the tree until it reaches a leaf node or finds the node containing the key.
 *
 * Parameters:
 *		BTreeHandle *tree: a pointer to the B-tree handle structure that contains information about the B-tree.
 *    int key: the key to be searched for in the B-tree.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/
BT_Node *findNodeByKey(BTreeHandle *tree, int key)
{
  BT_Node *current = tree->root;
  int index, fitOn;
  while (current != NULL && !current->isLeaf)
  {
    index = saBinarySearch(current->vals, key, &fitOn);
    index >= 0 ? fitOn += 1 : fitOn;
    current = current->children[fitOn];
  }
  return current;
}

/**************************************************************************************
 * Function Name: loadBtree
 *
 * Description:
 *		This function loads the B+ tree from the disk into memory.
 *
 * Parameters:
 *		BTreeHandle *tree: A pointer to the BTreeHandle struct which contains information about the B+ tree, such as the root node and the location of the root node on disk.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

RC loadBtree(BTreeHandle *tree)
{
  RC err;
  tree->root = NULL;
  if (tree->depth)
  {
    err = (readNode(&tree->root, tree, tree->whereIsRoot) != RC_OK) ? err : RC_OK;
    BT_Node **leftOnLevel = newArray(BT_Node *, tree->depth);
    int i = 0;
    do
    {
      leftOnLevel[i] = NULL;
      i++;
    } while (i < tree->depth);

    err = loadBtreeNodes(tree, tree->root, leftOnLevel, 0);
    free(leftOnLevel);
    return err;
  }
  return RC_OK;
}

/**************************************************************************************
 * Function Name: writeBtreeHeader
 *
 * Description:
 *		This function writes the B-tree header information to disk. The header information includes the root node's page number, the degree of the B-tree, and the depth of the B-tree.
 *
 * Parameters:
 *		BTreeHandle *tree: a pointer to the B-tree handle
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Karan Batra <kbatra3@hawk.iit.edu> (A20518491)
 **************************************************************************************/

RC writeBtreeHeader(BTreeHandle *tree)
{
  RC err;
  BM_BufferPool *bm = tree->mgmtData;
  BM_PageHandle *page = new (BM_PageHandle); // TODO free it
  RC_OK != (err = pinPage(bm, page, 0)) ? (freeit(1, page), err) : err;
  err = markDirty(bm, page);
  err != RC_OK ? (freeit(1, page), err) : err;

  err = unpinPage(bm, page);
  if (err != RC_OK)
  {
    freeit(1, page);
    return err;
  }

  forceFlushPool(bm);
  freeit(1, page);

  return RC_OK;
}

/**************************************************************************************
 * Function Name: insPropagateParent
 *
 * Description:
 *		Is responsible for propagating a newly inserted key and right child to the parent node of the left child.
 *
 * Parameters:
 *		BTreeHandle *tree: A pointer to the BTreeHandle struct that contains information about the B+ tree.
 *    BT_Node *left: A pointer to the left child node of the parent node.
 *    BT_Node *right: A pointer to the right child node of the parent node.
 *    int key: An integer value of the key to be propagated.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/
RC insPropagateParent(BTreeHandle *tree, BT_Node *left, BT_Node *right, int key)
{
  BT_Node *parent = left->parent;
  int index, i;
  (parent == NULL) ? (parent = createBTNode(tree->size, 0, tree->nextPage),
                      saInsertAt(parent->childrenPages, left->pageNum, 0),
                      parent->children[0] = left,
                      tree->nextPage++,
                      tree->whereIsRoot = parent->pageNum,
                      tree->numNodes++,
                      tree->depth++,
                      tree->root = parent,
                      writeBtreeHeader(tree))
                   : NULL;
  right->parent = parent;
  left->parent = parent;
  index = saInsert(parent->vals, key);
  if (index >= 0)
  {
    index += 1; // next position is the pointer
    saInsertAt(parent->childrenPages, right->pageNum, index);
    int i = parent->vals->fill;
    do
    {
      parent->children[i] = parent->children[i - 1];
      i--;
    } while (i > index);
    parent->children[index] = right;
    return writeNode(parent, tree);
  }
  else
  {
    BT_Node *overflowed = createBTNode(tree->size + 1, 0, -1);
    overflowed->vals->fill = parent->vals->fill;
    overflowed->childrenPages->fill = parent->childrenPages->fill;
    memcpy(overflowed->vals->elems, parent->vals->elems, SIZE_INT * parent->vals->fill);
    memcpy(overflowed->childrenPages->elems, parent->childrenPages->elems, SIZE_INT * parent->childrenPages->fill);
    memcpy(overflowed->children, parent->children, sizeof(BT_Node *) * parent->childrenPages->fill);
    index = saInsert(overflowed->vals, key);
    saInsertAt(overflowed->childrenPages, right->pageNum, index + 1);
    i = parent->childrenPages->fill;
    do
    {
      i--;
      overflowed->children[i + 1] = overflowed->children[i];
    } while (i > index + 1);

    overflowed->children[index + 1] = right;

    int leftFill = overflowed->vals->fill / 2;
    int rightFill = overflowed->vals->fill - leftFill;
    BT_Node *rightParent = createBTNode(tree->size, 0, tree->nextPage);
    tree->nextPage++;
    tree->numNodes++;

    parent->vals->fill = leftFill;
    parent->childrenPages->fill = leftFill + 1;
    int lptrsSize = parent->childrenPages->fill;
    memcpy(parent->vals->elems, overflowed->vals->elems, SIZE_INT * leftFill);
    memcpy(parent->childrenPages->elems, overflowed->childrenPages->elems, SIZE_INT * lptrsSize);
    memcpy(parent->children, overflowed->children, sizeof(BT_Node *) * lptrsSize);

    rightParent->vals->fill = rightFill;
    rightParent->childrenPages->fill = overflowed->childrenPages->fill - lptrsSize;
    int rptrsSize = rightParent->childrenPages->fill;
    memcpy(rightParent->vals->elems, overflowed->vals->elems + leftFill, SIZE_INT * rightFill);
    memcpy(rightParent->childrenPages->elems, overflowed->childrenPages->elems + lptrsSize, SIZE_INT * rptrsSize);
    memcpy(rightParent->children, overflowed->children + lptrsSize, sizeof(BT_Node *) * rptrsSize);

    destroyBTNode(overflowed);

    key = rightParent->vals->elems[0];
    saDeleteAt(rightParent->vals, 0, 1);

    rightParent->right = parent->right;
    (rightParent->right != NULL) ? (rightParent->right->left = rightParent) : 0;

    writeNode(parent, tree);
    writeNode(rightParent, tree);
    writeBtreeHeader(tree);

    return insPropagateParent(tree, parent, rightParent, key);

    parent->right = rightParent;
    rightParent->left = parent;
  }
}

/**************************************************************************************
 * Function Name: freeNodes
 *
 * Description:
 *		This function traverses a B-tree starting from the leftmost leaf node and frees all nodes in the tree, including the root.
 *
 * Parameters:
 *		BT_Node *root: a pointer to the root node of the B-tree to be freed.
 *
 * Return:
 *		void
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

void freeNodes(BT_Node *root)
{
  if (root == NULL)
  {
    return;
  }
  BT_Node *leaf = root;
  do
  {
    leaf = leaf->children[0];
  } while (!leaf->isLeaf);

  BT_Node *parent = leaf->parent;
  BT_Node *next;
  do
  {
    do
    {
      next = leaf->right;
      destroyBTNode(leaf);
      leaf = next;
    } while (leaf != NULL);
    if (parent == NULL)
    {
      break;
    }
    leaf = parent;
    parent = leaf->parent;
  } while (true);
}

/**************************************************************************************
 * Function Name: initIndexManager
 *
 * Description:
 *		This function initializes the index manager. It takes a pointer to implementation-specific data used by the index manager as an argument and returns an RC value indicating whether the operation was successful. In this implementation, the function does not perform any initialization and simply returns RC_OK.
 *
 * Parameters:
 *		void *mgmtData: a pointer to implementation-specific data used by the index manager
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

RC initIndexManager(void *mgmtData)
{
  return RC_OK;
}

/**************************************************************************************
 * Function Name: shutdownIndexManager
 *
 * Description:
 *		This function is used to shutdown the index manager. It performs any necessary clean-up operations before shutting down the index manager. It returns RC_OK upon successful shutdown.
 *
 * Parameters:
 *
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

RC shutdownIndexManager()
{
  return RC_OK;
}

/**************************************************************************************
 * Function Name: createBtree
 *
 * Description:
 *    This function creates a new B-tree index file with the given index ID, key type, and node size.
 *
 *
 * Parameters:
 *		char *idxId: a string representing the unique identifier of the index file to be created.
 *    DataType keyType: an enumeration type representing the data type of the key in the index file.
 *    int n: an integer representing the node size of the B-tree.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Karan Batra <kbatra3@hawk.iit.edu> (A20518491)
 **************************************************************************************/

RC createBtree(char *idxId, DataType keyType, int n)
{
  (n * 3 * SIZE_INT > PAGE_SIZE - BYTES_BT_HEADER_LEN) ? RC_IM_N_TO_LAGE : RC_OK;
  RC rc;
  RC_OK == (rc = createPageFile(idxId)) ? RC_OK : rc;

  SM_FileHandle *fHandle = new (SM_FileHandle); // TODO free it, Done below
  RC_OK != (rc = openPageFile(idxId, fHandle)) ? (free(fHandle), rc) : RC_OK;

  char *header = newCleanArray(char, PAGE_SIZE);
  char *ptr = header;
  memcpy(ptr, &n, SIZE_INT);
  ptr += SIZE_INT;
  memcpy(ptr, &keyType, SIZE_INT);
  ptr += SIZE_INT;
  int whereIsRoot = 0;
  memcpy(ptr, &whereIsRoot, SIZE_INT);
  ptr += SIZE_INT;
  int numNodes = 0;
  memcpy(ptr, &numNodes, SIZE_INT);
  ptr += SIZE_INT;
  int numEntries = 0;
  memcpy(ptr, &numEntries, SIZE_INT);
  ptr += SIZE_INT;
  int depth = 0;
  memcpy(ptr, &depth, SIZE_INT);
  int nextPage = 1;
  ptr += SIZE_INT;
  memcpy(ptr, &nextPage, SIZE_INT);

  RC_OK != (rc = writeBlock(0, fHandle, header)) ? (free(fHandle), free(header), rc) : RC_OK;

  free(header);
  rc = closePageFile(fHandle);
  free(fHandle);
  return rc;
}

/**************************************************************************************
 * Function Name: openBtree
 *
 * Description:
 *		This function opens an existing B+ tree and loads its metadata and root node into a BTreeHandle object. The metadata includes the size of the tree, the data type of its keys, the number of nodes, the number of entries, and the depth of the tree. The root node is loaded into the buffer pool that is managed by the BTreeHandle object.
 *
 * Parameters:
 *		BTreeHandle **tree: a double pointer to a BTreeHandle object that will be populated with the metadata and root node of the tree.
 *    char *idxId: a string representing the ID of the B+ tree to be opened.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Karan Batra <kbatra3@hawk.iit.edu> (A20518491)
 **************************************************************************************/
RC openBtree(BTreeHandle **tree, char *idxId)
{
  BTreeHandle *_tree = new (BTreeHandle);
  RC err;
  BM_BufferPool *bm = new (BM_BufferPool); // TODO: free it in close
  err = initBufferPool(bm, idxId, PER_IDX_BUF_SIZE, RS_LRU, NULL) ? (freeit(2, bm, _tree), err) : RC_OK;
  err != RC_OK ? err : 0;

  BM_PageHandle *pageHandle = new (BM_PageHandle); // TODO: free it, Done below
  err = pinPage(bm, pageHandle, 0);
  (err != RC_OK) ? (freeit(3, bm, pageHandle, _tree), err) : RC_OK;

  char *ptr = pageHandle->data;
  _tree->idxId = idxId;
  _tree->mgmtData = bm;

  memcpy(&_tree->size, ptr, SIZE_INT);
  ptr += SIZE_INT;
  memcpy(&_tree->keyType, ptr, SIZE_INT);
  ptr += SIZE_INT;
  memcpy(&_tree->whereIsRoot, ptr, SIZE_INT);
  ptr += SIZE_INT;
  memcpy(&_tree->numNodes, ptr, SIZE_INT);
  ptr += SIZE_INT;
  memcpy(&_tree->numEntries, ptr, SIZE_INT);
  ptr += SIZE_INT;
  memcpy(&_tree->depth, ptr, SIZE_INT);
  ptr += SIZE_INT;
  memcpy(&_tree->nextPage, ptr, SIZE_INT);

  err != RC_OK ? (freeit(1, pageHandle), closeBtree(_tree), err) : RC_OK;
  freeit(1, pageHandle);
  if ((err = loadBtree(_tree)))
  {
    closeBtree(_tree);
    return err;
  }
  *tree = _tree;
  return RC_OK;
}

/**************************************************************************************
 * Function Name: closeBtree
 *
 * Description:
 *		This function closes a given BTree index by shutting down the buffer pool used by the index, freeing all the nodes of the tree and the BTreeHandle structure.
 *
 * Parameters:
 *		BTreeHandle *tree: Pointer to the BTreeHandle structure that contains the information about the BTree index that needs to be closed.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/
RC closeBtree(BTreeHandle *tree)
{
  shutdownBufferPool(tree->mgmtData);
  freeNodes(tree->root);
  freeit(2, tree->mgmtData, tree);
  return RC_OK;
}

/**************************************************************************************
 * Function Name: deleteBtree
 *
 * Description:
 *		The function deletes the specified B+ tree file by checking if the file exists and then removing it. The function returns RC_FILE_NOT_FOUND if the file does not exist, RC_FS_ERROR if there is an error while removing the file, and RC_OK if the file is deleted successfully.
 *
 * Parameters:
 *		char *idxId: a pointer to a string representing the identifier of the B+ tree file to be deleted.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

RC deleteBtree(char *idxId)
{
  (access(idxId, F_OK) == -1) ? RC_FILE_NOT_FOUND : ((unlink(idxId) == -1) ? RC_FS_ERROR : RC_OK);
  return RC_OK;
}

/**************************************************************************************
 * Function Name: getNumNodes
 *
 * Description:
 *		This function retrieves the total number of nodes in a B+ tree.
 *
 * Parameters:
 *		BTreeHandle *tree: Pointer to the BTreeHandle structure for the B+ tree.
 *    int *result: Pointer to an integer variable where the result will be stored.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/
RC getNumNodes(BTreeHandle *tree, int *result)
{
  *result = tree->numNodes;
  return RC_OK;
}

/**************************************************************************************
 * Function Name: getNumEntries
 *
 * Description:
 *		This function returns the number of entries (key-value pairs) in the B+ tree pointed to by the given tree handle. The function stores the result in the memory location pointed to by result. The return value is RC_OK if the function completes successfully, and an appropriate error code otherwise.
 *
 * Parameters:
 *		BTreeHandle *tree: a pointer to a BTreeHandle structure representing the B+ tree
 *    int *result: a pointer to an integer where the function will store the number of entries in the B+ tree
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

RC getNumEntries(BTreeHandle *tree, int *result)
{
  *result = tree->numEntries;
  return RC_OK;
}

/**************************************************************************************
 * Function Name: getKeyType
 *
 * Description:
 *		This function is used to get the data type of keys that are stored in the B+ tree.
 *
 * Parameters:
 *		BTreeHandle *tree: A pointer to the BTreeHandle struct representing the B+ tree.
 *    DataType *result: A pointer to the variable where the data type of keys will be stored.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

RC getKeyType(BTreeHandle *tree, DataType *result)
{
  *result = tree->keyType;
  return RC_OK;
}

/**************************************************************************************
 * Function Name: findKey
 *
 * Description:
 *		This function searches for a given key in the B+ tree and returns the corresponding record ID.
 *
 * Parameters:
 *		BTreeHandle *tree: Pointer to a BTreeHandle struct that represents the B+ tree to search in.
 *    Value *key:  Pointer to a Value struct that represents the key to search for in the B+ tree.
 *    RID *result: Pointer to an RID struct that will hold the resulting record ID if the key is found.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

RC findKey(BTreeHandle *tree, Value *key, RID *result)
{
  int index, fitOn;
  BT_Node *leaf = findNodeByKey(tree, key->v.intV);
  index = saBinarySearch(leaf->vals, key->v.intV, &fitOn);
  return (index < 0) ? RC_IM_KEY_NOT_FOUND : (result->page = leaf->leafRIDPages->elems[index], result->slot = leaf->leafRIDSlots->elems[index], RC_OK);
}

/**************************************************************************************
 * Function Name: insertKey
 *
 * Description:
 *		This function inserts a key-value pair into the B-tree. If the key already exists in the B-tree, it returns an error. If the insertion causes a node to overflow, the function splits the node and propagates the splitting up the tree until it reaches the root.
 *
 * Parameters:
 *		BTreeHandle *tree: A pointer to the B-tree handle
 *    Value *key: A pointer to the key to be inserted
 *    RID rid: The RID (Record ID) associated with the key to be inserted.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Karan Batra <kbatra3@hawk.iit.edu> (A20518491)
 **************************************************************************************/

RC insertKey(BTreeHandle *tree, Value *key, RID rid)
{
  BT_Node *leaf = findNodeByKey(tree, key->v.intV);
  if (leaf == NULL)
  {
    leaf = createBTNode(tree->size, 1, tree->nextPage);
    tree->nextPage++;
    tree->whereIsRoot = leaf->pageNum;
    tree->numNodes++;
    tree->depth++;
    tree->root = leaf;
    writeBtreeHeader(tree);
  }
  int index, fitOn;
  index = saBinarySearch(leaf->vals, key->v.intV, &fitOn);
  if (index >= 0)
  {
    return RC_IM_KEY_ALREADY_EXISTS;
  }
  index = saInsert(leaf->vals, key->v.intV);
  if (index >= 0)
  {
    saInsertAt(leaf->leafRIDPages, rid.page, index);
    saInsertAt(leaf->leafRIDSlots, rid.slot, index);
  }
  else
  {
    BT_Node *overflowed = createBTNode(tree->size + 1, 1, -1);
    memcpy(overflowed->vals->elems, leaf->vals->elems, SIZE_INT * leaf->vals->fill);
    overflowed->vals->fill = leaf->vals->fill;
    memcpy(overflowed->leafRIDPages->elems, leaf->leafRIDPages->elems, SIZE_INT * leaf->leafRIDPages->fill);
    overflowed->leafRIDPages->fill = leaf->leafRIDPages->fill;
    memcpy(overflowed->leafRIDSlots->elems, leaf->leafRIDSlots->elems, SIZE_INT * leaf->leafRIDSlots->fill);
    overflowed->leafRIDSlots->fill = leaf->leafRIDSlots->fill;
    index = saInsert(overflowed->vals, key->v.intV);
    saInsertAt(overflowed->leafRIDPages, rid.page, index);
    saInsertAt(overflowed->leafRIDSlots, rid.slot, index);

    BT_Node *rightLeaf = createBTNode(tree->size, 1, tree->nextPage);
    tree->nextPage++;
    tree->numNodes++;
    int leftFill = ceil((float)overflowed->vals->fill / 2);
    int rightFill = overflowed->vals->fill - leftFill;

    leaf->vals->fill = leaf->leafRIDPages->fill = leaf->leafRIDSlots->fill = leftFill;
    memcpy(leaf->vals->elems, overflowed->vals->elems, SIZE_INT * leftFill);
    memcpy(leaf->leafRIDPages->elems, overflowed->leafRIDPages->elems, SIZE_INT * leftFill);
    memcpy(leaf->leafRIDSlots->elems, overflowed->leafRIDSlots->elems, SIZE_INT * leftFill);

    rightLeaf->vals->fill = rightLeaf->leafRIDPages->fill = rightLeaf->leafRIDSlots->fill = rightFill;
    memcpy(rightLeaf->vals->elems, overflowed->vals->elems + leftFill, SIZE_INT * rightFill);
    memcpy(rightLeaf->leafRIDPages->elems, overflowed->leafRIDPages->elems + leftFill, SIZE_INT * rightFill);
    memcpy(rightLeaf->leafRIDSlots->elems, overflowed->leafRIDSlots->elems + leftFill, SIZE_INT * rightFill);
    destroyBTNode(overflowed);
    (leaf->right != NULL) ? (leaf->right->left = rightLeaf) : 0;
    rightLeaf->left = leaf;
    rightLeaf->right = leaf->right;
    leaf->right = rightLeaf;

    rightLeaf->right != NULL ? rightLeaf->right->left = rightLeaf : 0;

    leaf->right = rightLeaf;
    rightLeaf->left = leaf;
    writeNode(rightLeaf, tree);
    writeNode(leaf, tree);
    insPropagateParent(tree, leaf, rightLeaf, rightLeaf->vals->elems[0]);
  }
  tree->numEntries++;
  writeBtreeHeader(tree);
  return RC_OK;
}

/**************************************************************************************
 * Function Name: deleteKey
 *
 * Description:
 *		This function deletes a key from a B-tree.
 *
 * Parameters:
 *		BTreeHandle *tree: a pointer to the BTreeHandle struct representing the B-tree to delete from
 *    Value *key: a pointer to the Value struct representing the key to delete from the B-tree
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

RC deleteKey(BTreeHandle *tree, Value *key)
{
  BT_Node *leaf = findNodeByKey(tree, key->v.intV);

  if (leaf != NULL)
  {
    int fitOn;
    int index = (leaf != NULL) ? saBinarySearch(leaf->vals, key->v.intV, &fitOn) : -1;

    index >= 0 ? (saDeleteAt(leaf->vals, index, 1), saDeleteAt(leaf->leafRIDPages, index, 1), saDeleteAt(leaf->leafRIDSlots, index, 1), tree->numEntries--, writeNode(leaf, tree), writeBtreeHeader(tree)) : false;
  }

  return RC_OK;
}

/**************************************************************************************
 * Function Name: openTreeScan
 *
 * Description:
 *		This function is used to open a scan on the B+ tree. It initializes the BT_ScanHandle and ScanMgmtInfo structs, sets the scan's starting point to the left-most leaf node of the tree, and assigns the scan handle to the provided pointer.
 *
 * Parameters:
 *		BTreeHandle *tree: A pointer to the BTreeHandle representing the B+ tree to be scanned.
 *    BT_ScanHandle **handle: A pointer to a BT_ScanHandle pointer that will store the newly created scan handle.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

RC openTreeScan(BTreeHandle *tree, BT_ScanHandle **handle)
{
  BT_ScanHandle *_handle = new (BT_ScanHandle);
  ScanMgmtInfo *scanInfo = new (ScanMgmtInfo);
  scanInfo->currentNode = tree->root;
  _handle->tree = tree;
  do
  {
    scanInfo->currentNode = scanInfo->currentNode->children[0];
  } while (!scanInfo->currentNode->isLeaf);

  scanInfo->elementIndex = 0;
  _handle->mgmtData = scanInfo;
  *handle = _handle;
  return RC_OK;
}

/**************************************************************************************
 * Function Name: closeTreeScan
 *
 * Description:
 *		This function closes an open scan on the B+ tree.
 *
 * Parameters:
 *		BT_ScanHandle *handle: A pointer to a BT_ScanHandle, which represents the scan that needs to be closed.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

RC closeTreeScan(BT_ScanHandle *handle)
{
  freeit(2, handle->mgmtData, handle);
  return RC_OK;
}

/**************************************************************************************
 * Function Name: nextEntry
 *
 * Description:
 *		This function retrieves the next entry from the B+ tree based on the scan handle and sets it to the RID *result. It moves the scan to the next entry in the B+ tree.
 *
 * Parameters:
 *		BT_ScanHandle *handle: a pointer to the scan handle of the B+ tree.
 *    RID *result: a pointer to the location where the result of the next entry should be stored.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

RC nextEntry(BT_ScanHandle *handle, RID *result)
{
  ScanMgmtInfo *scanInfo = handle->mgmtData;
  if (scanInfo->elementIndex >= scanInfo->currentNode->leafRIDPages->fill)
  {
    if (scanInfo->elementIndex == scanInfo->currentNode->vals->fill && scanInfo->currentNode->right == NULL)
    {
      return RC_IM_NO_MORE_ENTRIES;
    }
    else
    {
      scanInfo->currentNode = scanInfo->currentNode->right;
      scanInfo->elementIndex = 0;
    }
  }
  result->page = scanInfo->currentNode->leafRIDPages->elems[scanInfo->elementIndex];
  result->slot = scanInfo->currentNode->leafRIDSlots->elems[scanInfo->elementIndex];
  scanInfo->elementIndex++;
  return RC_OK;
}

/**************************************************************************************
 * Function Name: printTree
 *
 * Description:
 *		This function prints the B-tree in a readable format by traversing through the nodes and their elements. The function uses a helper function called printNodeHelper to print each node.
 *
 * Parameters:
 *		BTreeHandle *tree: Pointer to the B-tree that needs to be printed.
 *
 * Return:
 *		char *: Pointer to a character array that contains the readable format of the B-tree.
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

char *printTree(BTreeHandle *tree)
{
  int size = tree->numNodes * tree->size * 11 + tree->size + 14 + tree->numNodes;
  char *result = newCharArr(size);
  BT_Node *node = tree->root;
  int level = 0;
  while (node)
  {
    printNodeHelper(node, result);
    if (node->isLeaf)
    {
      node = node->right;
    }
    else
    {
      if (NULL == node->right)
      {
        BT_Node *temp = tree->root;
        int j = 0;
        do
        {
          temp = temp->children[0];
          j++;
        } while (j <= level);

        node = temp;
        level++;
      }
      else
      {
        node = node->right;
      }
    }
  }
  return result;
}

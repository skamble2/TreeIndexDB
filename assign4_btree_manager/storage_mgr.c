// Assignment 1 Storage Manager
// Group 20
// Soham Kamble (A20517098) <skamble2@hawk.iit.edu>
// Karan Batra (A20518491)  <kbatra3@hawk.iit.edu>
// Leily Nourbaksh (A20516826) <snoorbakhsh@hawk.iit.edu>

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "dberror.h"
#include "storage_mgr.h"

	#define FILE_READ_PERMISSION "r"
#define FILE_WRITE_PERMISSION "w+"
RC message;
FILE *pageF;

/**************************************************************************************
 * Function Name: initStorageManager
 *
 * Description:
 *		this function is used for initialization for storage manager. As defined below, the output
*		and inout of this function is void.
 *
 * Parameters:

 *		void
 *
 * Return:
 *		void
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

extern void initStorageManager(void)
{
	printf("Initializing storage manager!\n");
	pageF = NULL;
}

/**************************************************************************************
 * Function Name: createPageFile
 *
 * Description:
 *		Function is use to creating page file.
 *
 * Parameters:
 *		char *fileName : it is a file name
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

extern RC createPageFile(char *fileName)
{
	// opening file stream in write and read mode
	pageF = fopen(fileName, FILE_WRITE_PERMISSION);
	if (pageF == NULL)
	{
		message = RC_FILE_NOT_FOUND;
	}
	else
	{
		// creating empty page
		SM_PageHandle emptyPage = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
		// writing empty page
		fwrite(emptyPage, sizeof(char), PAGE_SIZE, pageF);
		// closing page file
		fclose(pageF);
		// free allocated space
		free(emptyPage);
		message = RC_OK;
	}
	return message;
}

/**************************************************************************************
 * Function Name: openPageFile
 *
 * Description:
 *		Function is use to open Page file.
 *
 * Parameters:
 *
 *		SM_FileHandle *fHandle: file pointer which contains information related to the file.
 *		char *fileName : it is a file name to be open
 *
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

extern RC openPageFile(char *fileName, SM_FileHandle *fHandle)
{
	// openinf file in read mode
	pageF = fopen(fileName, FILE_READ_PERMISSION);
	// checking if file opened successfully
	if (pageF == NULL)
	{
		return RC_FILE_HANDLE_NOT_INIT;
	}
	// setting the pointer to the starting position
	int start = fseek(pageF, 0, SEEK_SET);
	// setting filehandle's file name
	fHandle->fileName = fileName;
	// setting filehandle's current page position
	fHandle->curPagePos = start;
	// setting filehandle's total number of pages
	fHandle->totalNumPages = ftell(pageF) + 1;
	int close = fclose(pageF);
	message = (close != 0) ? RC_ERROR_WHILE_CLOSING_FILE : RC_OK;
	return message;
}

/**************************************************************************************
 * Function Name: closePageFile
 *
 * Description:
 *		Function is use to close page file.
 * Parameters:
 *
 *		SM_FileHandle *fHandle: file pointer which contains information related to the file.
 *
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

extern RC closePageFile(SM_FileHandle *fHandle)
{
	// opening file in read mode
	pageF = fopen(fHandle->fileName, FILE_READ_PERMISSION);
	// checking if file opened successfully or not
	if (pageF == NULL || pageF == 0)
	{
		printf("\nFile not found...");
		message = RC_FILE_NOT_FOUND;
	}
	else
	{
		// closing the page file
		int close = fclose(pageF);
		message = (close == 0) ? RC_OK : RC_ERROR_WHILE_CLOSING_FILE;
	}
	return message;
}

/**************************************************************************************
 * Function Name: destroyPageFile
 *
 * Description:
 *		Function is use to destroy page file.
 *
 * Parameters:
 *		char *fileName: it is a file to be deleted.
 *
 * Return:
 *		RC: return code
 *
 * Author:
		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 *
 **************************************************************************************/

extern RC destroyPageFile(char *fileName)
{
	printf("Inside destroy");
	// opening file in read mode
	pageF = fopen(fileName, FILE_READ_PERMISSION);
	// checking if file opened successfully or not
	if (pageF == NULL || pageF == 0)
	{
		printf("\nFile not found...");
		message = RC_FILE_NOT_FOUND;
	}
	else
	{
		// closing the page file
		int close = fclose(pageF);
		message = (close != 0) ? RC_ERROR_WHILE_CLOSING_FILE : ((remove(fileName) != 0) ? RC_ERROR_WHILE_REMOVING_FILE : RC_OK);
	}
	return message;
}

/**************************************************************************************
 * Function Name: readBlock
 *
 * Description:
 *		Function is use to read a memory block in a page on a specific location in memory.
 *
 * Parameters:
 *		int pageNum: start position of the page
 *		SM_FileHandle *fHandle: file pointer which contains information related to the file.
 *		SM_PageHandle memPage: the page handler which will point to the memory block that is to be stored in the file.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

extern RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	// opening file in read mode
	pageF = fopen(fHandle->fileName, FILE_READ_PERMISSION);
	if (pageF == NULL || pageF == 0)
	{
		printf("\nFile not found...");
		message = RC_FILE_NOT_FOUND;
	}
	else
	{
		if (pageNum < fHandle->totalNumPages || pageNum > 0)
		{
			// aligning the pointer with the file stream
			(fseek(pageF, (pageNum * PAGE_SIZE), SEEK_SET) != 0) ? RC_READ_NON_EXISTING_PAGE : (fread(memPage, sizeof(char), PAGE_SIZE, pageF), (fclose(pageF) != 0) ? RC_ERROR_WHILE_CLOSING_FILE : RC_OK);
		}
		else
		{
			message = RC_READ_NON_EXISTING_PAGE;
		}
	}
	return message;
}

/**************************************************************************************
 * Function Name: getBlockPos
 *
 * Description:
 *		Function is used to fetch the memory location of block.
 *
 * Parameters:
 *		SM_FileHandle *fHandle: file pointer which contains information related to the file.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

extern int getBlockPos(SM_FileHandle *fHandle)
{
	
	// opening file in read mode
	pageF = fopen(fHandle->fileName, FILE_READ_PERMISSION);
	if (pageF == NULL || pageF == 0)
	{
		printf("\nFile not found...");
		return RC_FILE_NOT_FOUND;
	}
	else
	{
		// updating the current position of page
		int curr = fHandle->curPagePos;
		int close = fclose(pageF);
		return (close != 0) ? RC_ERROR_WHILE_CLOSING_FILE : curr;
	}
}

/**************************************************************************************
 * Function Name: readFirstBlock
 *
 * Description:
 *		Function is use to read the first block in a file.
 *
 * Parameters:
 *		SM_FileHandle *fHandle: file pointer which contains information related to the file.
 *		SM_PageHandle memPage: the page handler which will point to the memory block that is to be stored in the file.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

extern RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	
	int first = 0;
	// assigning the values of the parameters
	message = readBlock(first, fHandle, memPage);
	return message;
}

/**************************************************************************************
 * Function Name: readPreviousBlock
 *
 * Description:
 *		Function is used to read the previous block in memory.
 *
 * Parameters:
 *		SM_FileHandle *fHandle: file pointer which contains information related to the file.
 *		SM_PageHandle memPage: the page handler which will point to the memory block that is to be stored in the file.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

extern RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	// set the pointer to the current page position and subtract by 1 in order to read the previous block
	int previousPageNumber = (fHandle->curPagePos / PAGE_SIZE) - 1;
	(previousPageNumber > 0) ? readBlock(previousPageNumber, fHandle, memPage) : RC_ERROR_INVALID_PAGENUMBER;
}

/**************************************************************************************
 * Function Name: readCurrentBlock
 *
 * Description:
 *		Function is used to read the current block in the current memory location
 *
 * Parameters:
 *		SM_FileHandle *fHandle: file pointer which contains information related to the file.
 *		SM_PageHandle memPage: the page handler which will point to the memory block that is to be stored in the file.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

extern RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	// set the pointer to the current page position in order to read the current block
	// set the pointer to the current page position in order to read the current block
	int currentPageNumber = fHandle->curPagePos / PAGE_SIZE;
	(currentPageNumber > 0) ? readBlock(currentPageNumber, fHandle, memPage) : RC_ERROR_INVALID_PAGENUMBER;
}

/**************************************************************************************
 * Function Name: readNextBlock
 *
 * Description:
 *		Function is used to read the next block in memory.
 *
 * Parameters:
 *		SM_FileHandle *fHandle: file pointer which contains information related to the file.
 *		SM_PageHandle memPage: the page handler which will point to the memory block that is to be stored in the file.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

extern RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	// set the pointer to the current page position and add the value by 1 in order to read the next block
	int nextPageNumber = (fHandle->curPagePos / PAGE_SIZE) + 1;
	return (nextPageNumber > 0) ? readBlock(nextPageNumber, fHandle, memPage) : RC_ERROR_INVALID_PAGENUMBER;
}

/**************************************************************************************
 * Function Name: readLastBlock
 *
 * Description:
 *		Function is used to read the last block in memory.
 *
 * Parameters:
 *		SM_FileHandle *fHandle: file pointer which contains information related to the file.
 *		SM_PageHandle memPage: the page handler which will point to the memory block that is to be stored in the file.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

extern RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	// setting the pointer to the last page number and subtract it by 1
	int lastPageNumber = fHandle->totalNumPages - 1;
	return (lastPageNumber > 0) ? readBlock(lastPageNumber, fHandle, memPage) : RC_ERROR_INVALID_PAGENUMBER;
}

/**************************************************************************************
 * Function Name: writeBlock
 *
 * Description:
 *		Function is use to write a memory block in a page on a specific location.
 *
 * Parameters:
 *		int pageNum: start position of the page
 *		SM_FileHandle *fHandle: file pointer which contains information related to the file.
 *		SM_PageHandle memPage: the page handler which will point to the memory block that is to be stored in the file.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Karan Batra <kbatra3@hawk.iit.edu> (A20518491)
 **************************************************************************************/

extern RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	if (pageNum > fHandle->totalNumPages || pageNum < 0)
		return RC_WRITE_FAILED;

	pageF = fopen(fHandle->fileName, "r+");

	if (pageF == NULL)
		return RC_FILE_NOT_FOUND;

	int startPosition = pageNum * PAGE_SIZE;

	if (pageNum == 0)
	{
		fseek(pageF, startPosition, SEEK_SET);
		int i = 0;
		do
		{
			if (feof(pageF))
				appendEmptyBlock(fHandle);
			fputc(memPage[i], pageF);
			i++;
		} while (i < PAGE_SIZE);

		fHandle->curPagePos = ftell(pageF);

		fclose(pageF);
	}
	else
	{
		fHandle->curPagePos = startPosition;
		fclose(pageF);
		writeCurrentBlock(fHandle, memPage);
	}
	return RC_OK;
}

/**************************************************************************************
 * Function Name: writeCurrentBlock
 *
 * Description:
 *		This function will write one block in the dist at the current position.
 *
 * Parameters:
 *		SM_FileHandle *fHandle: file pointer which contains information related to the file.
 *		SM_PageHandle memPage: the page handler which will point to the memory block that is to be stored in the file.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *      Karan Batra <kbatra3@hawk.iit.edu> (A20518491)
 **************************************************************************************/

extern RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	pageF = fopen(fHandle->fileName, "r+");

	if (pageF == NULL)
		return RC_FILE_NOT_FOUND;

	appendEmptyBlock(fHandle);

	fseek(pageF, fHandle->curPagePos, SEEK_SET);

	fwrite(memPage, sizeof(char), strlen(memPage), pageF);

	fHandle->curPagePos = ftell(pageF);

	fclose(pageF);
	return RC_OK;
}

/**************************************************************************************
 * Function Name: appendEmptyBlock
 *
 * Description:
 *		The functions will append an empty block at the end of the disk
 *
 * Parameters:
 *		SM_FileHandle *fHandle: file pointer which contains information related to the file.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *      Karan Batra <kbatra3@hawk.iit.edu> (A20518491)
 **************************************************************************************/

extern RC appendEmptyBlock(SM_FileHandle *fHandle)
{
	// create an empty block of same size as PAGE SIZE
	SM_PageHandle emptyBlock = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
	int seek_status = fseek(pageF, 0, SEEK_END);
	return (seek_status != 0) ? (free(emptyBlock), RC_WRITE_FAILED) :
                            (fwrite(emptyBlock, sizeof(char), PAGE_SIZE, pageF), RC_OK);
	free(emptyBlock);
	// update the total number of pages
	fHandle->totalNumPages++;
	message = RC_OK;
	return message;
}

/**************************************************************************************
 * Function Name: ensureCapacity
 *
 * Description:
 *		To ensure the number of pages are enough if not then increase the size to numberOfPages.
 *
 * Parameters:
 *		int numberOfPages: the desired number of pages available
 *		SM_FileHandle *fHandle: file pointer which contains information related to the file.
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Karan Batra <kbatra3@hawk.iit.edu> (A20518491)
 **************************************************************************************/

extern RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle)
{
	//// Open file stream in append mode to append the data at the end of file.
	pageF = fopen(fHandle->fileName, "a");
	if (pageF == NULL || pageF == 0)
	{
		printf("\nFile not found...");
		message = RC_FILE_NOT_FOUND;
	}
	else
	{
		// check if number of pages exceeds the total number of pages
		do {
    appendEmptyBlock(fHandle);
} while (numberOfPages > fHandle->totalNumPages);
		// closing file stream
		fclose(pageF);
		message = RC_OK;
}
}
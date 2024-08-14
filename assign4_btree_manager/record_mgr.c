// Assignment 3 Record Manager
// Group 20
// Soham Kamble (A20517098) <skamble2@hawk.iit.edu>
// Karan Batra (A20518491)  <kbatra3@hawk.iit.edu>
// Leily Nourbaksh (A20516826) <snoorbakhsh@hawk.iit.edu>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "record_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"

typedef struct RecordManager
{
	BM_PageHandle page_handle;
	BM_BufferPool buffer_pool;
	RID record_id;
	Expr *condition;
	int tuples_count;
	int free_page;
	int scan_count;
} RecordManager;

const int max_no_of_pages = 100;
const int attribute_size = 15;

RecordManager *recordManager;

/**************************************************************************************
 * Function Name: findFreeSlot
 *
 * Description:
 *		This function returns a free slot within a page
 *
 * Parameters:
 *		char *data: data
 *		int record_size: size of the record
 *
 * Return:
 *		-1
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

int findFreeSlot(char *data, int record_size)
{
	int i, total_slots = PAGE_SIZE / record_size;

	i = 0;
	do
	{
		if (!(data[i * record_size] == '+'))
		{
			return i;
		}
		i++;
	} while (i < total_slots);

	return -1;
}

/**************************************************************************************
 * Function Name: initRecordManager
 *
 * Description:
 *		This function initializes the Record Manager
 *
 * Parameters:
 *		void *mgmtData:
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

extern RC initRecordManager(void *mgmtData)
{
	initStorageManager();
	return RC_OK;
}

/**************************************************************************************
 * Function Name: shutdownRecordManager
 *
 * Description:
 *		This functions shuts down the Record Manager
 *
 * Parameters:
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

extern RC shutdownRecordManager()
{
	recordManager = NULL;
	free(recordManager);
	return RC_OK;
}

/**************************************************************************************
 * Function Name: createTable
 *
 * Description:
 *		This function creates a TABLE with table name "name" having schema specified by "schema"
 *
 * Parameters:
 *		char *name: name of the table
 *		Schema *schema: schema of the table
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

extern RC createTable(char *name, Schema *schema)
{
	recordManager = (RecordManager *)malloc(sizeof(RecordManager));

	initBufferPool(&recordManager->buffer_pool, name, max_no_of_pages, RS_LRU, NULL);

	char data[PAGE_SIZE];
	char *page_handle = data;

	int res, k;

	*(int *)page_handle = 0;

	page_handle = page_handle + sizeof(int);

	*(int *)page_handle = 1;

	page_handle = page_handle + sizeof(int);

	*(int *)page_handle = schema->numAttr;

	page_handle = page_handle + sizeof(int);

	*(int *)page_handle = schema->keySize;

	page_handle = page_handle + sizeof(int);

	k = 0;
	do
	{
		strncpy(page_handle, schema->attrNames[k], attribute_size);
		page_handle = page_handle + attribute_size;

		*(int *)page_handle = (int)schema->dataTypes[k];

		page_handle = page_handle + sizeof(int);

		*(int *)page_handle = (int)schema->typeLength[k];

		page_handle = page_handle + sizeof(int);

		k++;
	} while (k < schema->numAttr);

	SM_FileHandle fileHandle;

	res = createPageFile(name);
	res = (openPageFile(name, &fileHandle) != RC_OK) ? res : RC_OK;

	do
	{
		res = writeBlock(0, &fileHandle, data);
	} while (res == RC_WRITE_FAILED);

	res = closePageFile(&fileHandle);
	(res != RC_OK) ? res : openPageFile(name, &fileHandle);

	return RC_OK;
}

/**************************************************************************************
 * Function Name: openTable
 *
 * Description:
 *		Function is used to open a table
 *
 * Parameters:
 *		RM_TableData *rel: table to open
 *		char *name: name of the table
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/
extern RC openTable(RM_TableData *rel, char *name)
{
	SM_PageHandle page_handle;

	int attribute_count, k;

	rel->mgmtData = recordManager;
	rel->name = name;

	pinPage(&recordManager->buffer_pool, &recordManager->page_handle, 0);

	page_handle = (char *)recordManager->page_handle.data;

	recordManager->tuples_count = *(int *)page_handle, page_handle += sizeof(int);

	recordManager->free_page = *(int *)page_handle, page_handle += sizeof(int);

	attribute_count = *(int *)page_handle, page_handle += sizeof(int);

	Schema *schema;

	schema = (Schema *)malloc(sizeof(Schema));

	schema->numAttr = attribute_count;
	schema->attrNames = (char **)malloc(sizeof(char *) * attribute_count);
	schema->dataTypes = (DataType *)malloc(sizeof(DataType) * attribute_count);
	schema->typeLength = (int *)malloc(sizeof(int) * attribute_count);
	k = 0;
	do
	{
		schema->attrNames[k] = (char *)malloc(attribute_size);
		k++;
	} while (k < attribute_count);

	int j = 0;
	do
	{
		strncpy(schema->attrNames[j], page_handle, attribute_size);
		page_handle = page_handle + attribute_size;

		schema->dataTypes[j] = *(int *)page_handle;
		page_handle = page_handle + sizeof(int);

		schema->typeLength[j] = *(int *)page_handle;
		page_handle = page_handle + sizeof(int);

		j++;
	} while (j < schema->numAttr);

	rel->schema = schema;

	unpinPage(&recordManager->buffer_pool, &recordManager->page_handle);
	forcePage(&recordManager->buffer_pool, &recordManager->page_handle);

	return RC_OK;
}

/**************************************************************************************
 * Function Name: closeTable
 *
 * Description:
 *		This function closes the table referenced by "rel"
 *
 * Parameters:
 *		RM_TableData *rel: table to be closed
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

extern RC closeTable(RM_TableData *rel)
{

	RecordManager *recordManager = rel->mgmtData;

	shutdownBufferPool(&recordManager->buffer_pool);

	return RC_OK;
}

/**************************************************************************************
 * Function Name: deleteTable
 *
 * Description:
 *		 This function deletes the table having table name "name"
 *
 * Parameters:
 *		char *name: name of table to be deleted
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

extern RC deleteTable(char *name)
{
	destroyPageFile(name);
	return RC_OK;
}

/**************************************************************************************
 * Function Name: getNumTuples
 *
 * Description:
 *		This function returns the number of tuples (records) in the table referenced by "rel"
 *
 * Parameters:
 *		RM_TableData *rel: table referenced
 *
 * Return:
 *		tuples_count: number of tuples
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

extern int getNumTuples(RM_TableData *rel)
{
	RecordManager *recordManager = rel->mgmtData;
int numTuples = recordManager->tuples_count;
return numTuples;

}

/**************************************************************************************
 * Function Name: insertRecord
 *
 * Description:
 *		This function inserts a new record in the table referenced by "rel" and updates the 'record' parameter with the Record ID of he newly inserted record
 *
 * Parameters:
 *		RM_TableData *rel: table reference
 *		Record *record: record to be inserted
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Karan Batra <kbatra3@hawk.iit.edu> (A20518491)
 **************************************************************************************/
// ******** RECORD FUNCTIONS ******** //

extern RC insertRecord(RM_TableData *rel, Record *record)
{

	RecordManager *recordManager = rel->mgmtData;

	RID *record_id = &record->id;

	char *data, *slot_pointer;

	int record_size = getRecordSize(rel->schema);

	record_id->page = recordManager->free_page;

	pinPage(&recordManager->buffer_pool, &recordManager->page_handle, record_id->page);

	data = recordManager->page_handle.data;

	record_id->slot = findFreeSlot(data, record_size);

	while (record_id->slot < 0)
	{

		unpinPage(&recordManager->buffer_pool, &recordManager->page_handle);

		record_id->page++;

		pinPage(&recordManager->buffer_pool, &recordManager->page_handle, record_id->page);

		data = recordManager->page_handle.data;

		record_id->slot = findFreeSlot(data, record_size);
	}

	slot_pointer = (char*)recordManager->page_handle.data + (record_id->slot * record_size);

	*slot_pointer = '+';

	memcpy(slot_pointer + 1, record->data + 1, record_size - 1);

	markDirty(&recordManager->buffer_pool, &recordManager->page_handle);
	recordManager->tuples_count++;

	unpinPage(&recordManager->buffer_pool, &recordManager->page_handle);


	return RC_OK;
}

/**************************************************************************************
 * Function Name: deleteRecord
 *
 * Description:
 *		This function deletes a record having Record ID "id" in the table referenced by "rel"
 *
 * Parameters:
 *		RM_TableData *rel: table reference
 *		RID id: id of record
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Karan Batra <kbatra3@hawk.iit.edu> (A20518491)
 **************************************************************************************/

extern RC deleteRecord(RM_TableData *rel, RID id)
{

	RecordManager *recordManager = rel->mgmtData;
	PageNumber page = id.page;

	pinPage(&recordManager->buffer_pool, &(recordManager->page_handle), page);

	recordManager->free_page = page;

	char *data_pointer = recordManager->page_handle.data;

	int record_size = getRecordSize(rel->schema);

	data_pointer = data_pointer + (id.slot * record_size);

	*data_pointer = '-';

	markDirty(&recordManager->buffer_pool, &(recordManager->page_handle));

	unpinPage(&recordManager->buffer_pool, &(recordManager->page_handle));


	return RC_OK;
}

/**************************************************************************************
 * Function Name: updateRecord
 *
 * Description:
 *		This function updates a record referenced by "record" in the table referenced by "rel"
 *
 * Parameters:
 *		RM_TableData *rel: table referenced
 *		Record *record: record to update
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Karan Batra <kbatra3@hawk.iit.edu> (A20518491)
 **************************************************************************************/
extern RC updateRecord(RM_TableData *rel, Record *record)
{

	RecordManager *recordManager = rel->mgmtData;

	RID id = record->id;

	pinPage(&recordManager->buffer_pool, &(recordManager->page_handle), id.page);

	char *data = recordManager->page_handle.data;

	int record_size = getRecordSize(rel->schema);

	data += (id.slot * record_size);

	*data = '+';

	memcpy(++data, record->data + 1, record_size - 1);

	markDirty(&recordManager->buffer_pool, &(recordManager->page_handle));

	unpinPage(&recordManager->buffer_pool, &(recordManager->page_handle));

	return RC_OK;

}

/**************************************************************************************
 * Function Name: getRecord
 *
 * Description:
 *		This function retrieves a record having Record ID "id" in the table referenced by "rel". The res record is stored in the location referenced by "record"
 *
 * Parameters:
 *		RM_TableData *rel: table reference
 *		RID id: record identifier
 *		Record *record: record to retrieve
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Karan Batra <kbatra3@hawk.iit.edu> (A20518491)
 **************************************************************************************/

extern RC getRecord(RM_TableData *rel, RID id, Record *record)
{

	RecordManager *recordManager = rel->mgmtData;

	pinPage(&recordManager->buffer_pool, &recordManager->page_handle, id.page);

	int record_size = getRecordSize(rel->schema);
	char *data_pointer = recordManager->page_handle.data + (id.slot * record_size);


	if (*data_pointer == '+')
	{
		char *data = record->data;
record->id = id;
memcpy(++data, data_pointer + 1, record_size - 1);

	}
	else
	{
		return RC_RM_NO_TUPLE_WITH_GIVEN_RID;
	}

	unpinPage(&recordManager->buffer_pool, &recordManager->page_handle);

	return RC_OK;
}

/**************************************************************************************
 * Function Name: startScan
 *
 * Description:
 *		This function scans all the records using the condition
 *
 * Parameters:
 *		RM_TableData *rel: table reference
 *		RM_ScanHandle *scan: scan handle
 *		Expr *condition: condition
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/
// ******** SCAN FUNCTIONS ******** //

extern RC startScan(RM_TableData *rel, RM_ScanHandle *scan, Expr *condition)
{

	condition ? RC_OK : RC_SCAN_CONDITION_NOT_FOUND;

	openTable(rel, "ScanTable");

	RecordManager *scan_manager;
	RecordManager *table_manager;

	scan_manager = (RecordManager *)malloc(sizeof(RecordManager));

	scan->mgmtData = scan_manager;

	scan_manager->record_id.page = 1;

	scan_manager->record_id.slot = 0;

	scan_manager->scan_count = 0;

	scan_manager->condition = condition;

	table_manager = rel->mgmtData;

	table_manager->tuples_count = attribute_size;

	scan->rel = rel;

	return RC_OK;
}

/**************************************************************************************
 * Function Name: next
 *
 * Description:
 *		This function scans each record in the table and stores the res record (record satisfying the condition) in the location pointed by  'record'.
 *
 * Parameters:
 *		RM_ScanHandle *scan: scan handle
 *		Record *record: record at current position
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

extern RC next(RM_ScanHandle *scan, Record *record)
{

	RecordManager *scan_manager = scan->mgmtData;
	RecordManager *table_manager = scan->rel->mgmtData;
	Schema *schema = scan->rel->schema;

	(scan_manager == NULL || scan_manager->condition == NULL) ? RC_SCAN_CONDITION_NOT_FOUND : RC_OK;

	Value *res = (Value *)malloc(sizeof(Value));

	char *data;

	int record_size = getRecordSize(schema);

	int total_slots = PAGE_SIZE / record_size;

	int scan_count = scan_manager->scan_count;

	int tuples_count = table_manager->tuples_count;

	tuples_count == 0 ? RC_RM_NO_MORE_TUPLES : RC_OK;





	do
	{
		if (scan_count <= 0) {
    scan_manager->record_id.page = 1;
    scan_manager->record_id.slot = 0;
} else {
    scan_manager->record_id.slot = (scan_manager->record_id.slot >= total_slots) ? 0 : (scan_manager->record_id.slot + 1);
    if (scan_manager->record_id.slot == 0) {
        scan_manager->record_id.page++;
    }
}


		pinPage(&table_manager->buffer_pool, &scan_manager->page_handle, scan_manager->record_id.page);

data = scan_manager->page_handle.data;

data = data + (scan_manager->record_id.slot * record_size);

record->id = scan_manager->record_id;

char *data_pointer = record->data;

*data_pointer = '-';

memcpy(++data_pointer, data + 1, record_size - 1);

scan_manager->scan_count++;
scan_count++;

evalExpr(record, schema, scan_manager->condition, &res);


		if (res->v.boolV)
		{
			unpinPage(&table_manager->buffer_pool, &scan_manager->page_handle);
			return RC_OK;
		}

	} while (++scan_count <= tuples_count);

	scan_manager->record_id.page = 1;
	scan_manager->record_id.slot = 0;
	scan_manager->scan_count = 0;

	unpinPage(&table_manager->buffer_pool, &scan_manager->page_handle);

	return RC_RM_NO_MORE_TUPLES;
}

/**************************************************************************************
 * Function Name: closeScan
 *
 * Description:
 *		This function closes the scan operation.
 *
 * Parameters:
 *		RM_ScanHandle *scan: scan handle
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

extern RC closeScan(RM_ScanHandle *scan)
{
	RecordManager *scan_manager = scan->mgmtData;
	RecordManager *recordManager = scan->rel->mgmtData;

	(scan_manager->scan_count > 0) ? (unpinPage(&recordManager->buffer_pool, &scan_manager->page_handle),
									  scan_manager->scan_count = 0,
									  scan_manager->record_id.page = 1,
									  scan_manager->record_id.slot = 0)
								   : 0;

	scan->mgmtData = NULL;
	free(scan->mgmtData);

	return RC_OK;
}

/**************************************************************************************
 * Function Name: getRecordSize
 *
 * Description:
 *		This function returns the record size of the schema referenced by "schema"
 *
 * Parameters:
 *		Schema *schema: schema reference
 *
 * Return:
 *		size: size of the record
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/
// ******** SCHEMA FUNCTIONS ******** //
extern int getRecordSize(Schema *schema)
{
	int size = 0, i;

	i = 0;
	do
	{
		size += (schema->dataTypes[i] == DT_STRING) ? schema->typeLength[i] : (schema->dataTypes[i] == DT_INT) ? sizeof(int)
																		  : (schema->dataTypes[i] == DT_FLOAT) ? sizeof(float)
																		  : (schema->dataTypes[i] == DT_BOOL)  ? sizeof(bool)
																											   : 0;

	} while (++i < schema->numAttr);
	return ++size;
}

/**************************************************************************************
 * Function Name: createSchema
 *
 * Description:
 *		This function creates a new schema
 *
 * Parameters:
 *		int numAttr: number of attributes
 *		char **attrNames: name of the attributes
 *		DataType *dataTypes: data types
 *		int *typeLength: type length
 *		int keySize: key size
 *		int *keys: keys
 *
 * Return:
 *		schema: resulting schema
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

extern Schema *createSchema(int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys)
{
	Schema *schema = (Schema *)malloc(sizeof(Schema));
	*schema = (Schema){
		.numAttr = numAttr,
		.attrNames = attrNames,
		.dataTypes = dataTypes,
		.typeLength = typeLength,
		.keySize = keySize,
		.keyAttrs = keys};

	return schema;
}

/**************************************************************************************
 * Function Name: freeSchema
 *
 * Description:
 *		This function removes a schema from memory and de-allocates all the memory space allocated to the schema.
 *
 * Parameters:
 *		Schema *schema: schema to be deleted
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/
extern RC freeSchema(Schema *schema)
{
	free(schema);
	return RC_OK;
}

/**************************************************************************************
 * Function Name: createRecord
 *
 * Description:
 *		This function creates a new record in the schema referenced by "schema"
 *
 * Parameters:
 *		Record **record: record to be created
 *		Schema *schema: schema of the record
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Karan Batra <kbatra3@hawk.iit.edu> (A20518491)
 **************************************************************************************/
// ******** DEALING WITH RECORDS AND attr VALUES ******** //

extern RC createRecord(Record **record, Schema *schema)
{
	Record *new_record = malloc(sizeof(Record));
	new_record->id.page = new_record->id.slot = -1;

	int record_size = getRecordSize(schema);
char* data = calloc(record_size, sizeof(char));
data[0] = '-';
new_record->data = data;

	*record = new_record;

	return RC_OK;
}

/**************************************************************************************
 * Function Name: attrOffset
 *
 * Description:
 *		This function sets the offset (in bytes) from initial position to the specified attr of the record into the 'res' parameter passed through the function
 *
 * Parameters:
 *		Schema *schema: schema of the record
 *		int attrNum: number of attributes
 *		int *res: result
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Karan Batra <kbatra3@hawk.iit.edu> (A20518491)
 **************************************************************************************/
RC attrOffset(Schema *schema, int attrNum, int *res)
{
	int i;
	*res = 1;

	for (i = 0; i < attrNum; i++) {
    *res = *res + ((schema->dataTypes[i] == DT_STRING) ? schema->typeLength[i]
                                                      : (schema->dataTypes[i] == DT_INT) ? sizeof(int)
                                                      : (schema->dataTypes[i] == DT_FLOAT) ? sizeof(float)
                                                      : (schema->dataTypes[i] == DT_BOOL) ? sizeof(bool)
                                                      : 0);
}
	return RC_OK;
}
/**************************************************************************************
 * Function Name: freeRecord
 *
 * Description:
 *		This function removes the record from the memory.
 *
 * Parameters:
 *		Record *record: record to be removed
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Karan Batra <kbatra3@hawk.iit.edu> (A20518491)
 **************************************************************************************/

extern RC freeRecord(Record *record)
{
	free(record);
	return RC_OK;
}

/**************************************************************************************
 * Function Name: getAttr
 *
 * Description:
 *		This function retrieves an attr from the given record in the specified schema
 *
 * Parameters:
 *		Record *record: record of the attribute
 *		Schema *schema: schema of attribute
 *		Value **value: value of the attribute
 *		int attrNum: number of attribute
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Karan Batra <kbatra3@hawk.iit.edu> (A20518491)
 **************************************************************************************/

extern RC getAttr(Record *record, Schema *schema, int attrNum, Value **value)
{
	int offset = 0;

	attrOffset(schema, attrNum, &offset);

Value *attr = (Value *)malloc(sizeof(Value));

char *data_pointer = record->data;

data_pointer += offset;

if (attrNum == 1) {
	schema->dataTypes[attrNum] = 1;
} else {
	schema->dataTypes[attrNum] = schema->dataTypes[attrNum];
}






	if (schema->dataTypes[attrNum] == DT_STRING) {
int length = schema->typeLength[attrNum];
attr->v.stringV = (char *)malloc(length + 1);
strncpy(attr->v.stringV, data_pointer, length);
attr->v.stringV[length] = '\0';
attr->dt = DT_STRING;
} else {
if (schema->dataTypes[attrNum] == DT_INT) {
int value = 0;
memcpy(&value, data_pointer, sizeof(int));
attr->v.intV = value;
attr->dt = DT_INT;
} else {
if (schema->dataTypes[attrNum] == DT_FLOAT) {
float value;
memcpy(&value, data_pointer, sizeof(float));
attr->v.floatV = value;
attr->dt = DT_FLOAT;
} else {
if (schema->dataTypes[attrNum] == DT_BOOL) {
bool value;
memcpy(&value, data_pointer, sizeof(bool));
attr->v.boolV = value;
attr->dt = DT_BOOL;
} else {
printf("Serializer not defined for the given datatype. \n");
}
}
}
}

	*value = attr;
	return RC_OK;
}

/**************************************************************************************
 * Function Name: setAttr
 *
 * Description:
 *		This function sets the attr value in the record in the specified schema
 *
 * Parameters:
 *		Record *record: record to set
 *		Schema *schema: schema to set
 *		int attrNum: attribute number
 *		Value *value: value to set
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Karan Batra <kbatra3@hawk.iit.edu> (A20518491)
 **************************************************************************************/

extern RC setAttr(Record *record, Schema *schema, int attrNum, Value *value)
{
	int offset = 0;

	attrOffset(schema, attrNum, &offset);
char *data_pointer = &(record->data[offset]);

	int data_type = schema->dataTypes[attrNum];
int data_length = schema->typeLength[attrNum];

if (data_type == DT_STRING)
{
strncpy(data_pointer, value->v.stringV, data_length);
data_pointer += data_length;
}
else if (data_type == DT_INT)
{
*(int *)data_pointer = value->v.intV;
data_pointer += sizeof(int);
}
else if (data_type == DT_FLOAT)
{
*(float *)data_pointer = value->v.floatV;
data_pointer += sizeof(float);
}
else if (data_type == DT_BOOL)
{
*(bool *)data_pointer = value->v.boolV;
data_pointer += sizeof(bool);
}
else
{
printf("Serializer not defined for the given datatype. \n");
}
	return RC_OK;
}
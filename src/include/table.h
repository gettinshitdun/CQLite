#ifndef TABLE_H
#define TABLE_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#define TABLE_MAX_PAGES 50000000

#define MAX_TYPE_LEN 16
#define MAX_NAME_LEN 64
#define MAX_TBL_NAME 64
#define MAX_SQL_LEN 256

typedef struct {
    uint32_t rowid;
    char     type[MAX_TYPE_LEN];     // "table", "index", etc.
    char     name[MAX_NAME_LEN];     // object name
    char     tbl_name[MAX_TBL_NAME]; // table name for index, same as name for tables
    uint32_t root_page;              // root page of table/index B-tree
    char     sql[MAX_SQL_LEN];       // CREATE TABLE/INDEX statement
} SchemaRow;

typedef struct {
    int      file_descriptor;
    uint32_t file_length;
    void*    pages[TABLE_MAX_PAGES];
    uint32_t num_pages;
} Pager;

typedef struct {
    Pager*   pager;
    uint32_t root_page_num;
} Table;

typedef struct {
    Table*   table;
    uint32_t page_num;
    uint32_t cell_num;
    bool     end_of_table;
} Cursor;

typedef struct {
    uint32_t leaf_nodes;
    uint32_t internal_nodes;
    uint32_t total_pages;
    uint32_t max_depth;
} BTreeStats;

Cursor* table_start(Table* table);
Cursor* table_find(Table* table, uint32_t key);

Table* db_open(const char* filename);
Pager* pager_open(const char* filename);
void   db_close(Table* table);
void   serialize_schema_row(SchemaRow* source, void* destination);
void   deserialize_schema_row(void* source, SchemaRow* destination);
void*  cursor_value(Cursor* cursor);
void   cursor_advance(Cursor* cursor);
void   print_schema_row(SchemaRow* row);

extern const uint32_t TABLE_MAX_ROWS;
extern const uint32_t LEAF_NODE_MAX_CELLS;
void*                 get_page(Pager* pager, uint32_t page_num);

Pager* pager_open(const char* filename);

void pager_flush(Pager* pager, uint32_t page_num);

typedef enum { NODE_INTERNAL, NODE_LEAF } NodeType;

uint32_t* leaf_node_num_cells(void* node);
void*     leaf_node_cell(void* node, uint32_t cell_num);
uint32_t* leaf_node_key(void* node, uint32_t cell_num);
Cursor*   leaf_node_find(Table* table, uint32_t page_num, uint32_t key);

void* leaf_node_value(void* node, uint32_t cell_num);
void  initialize_leaf_node(void* node);
void  leaf_node_insert(Cursor* cursor, uint32_t key, SchemaRow* value);

void print_constants();
void print_tree(Pager* pager, uint32_t page_num, uint32_t indentation_level);

void     set_node_type(void* node, NodeType type);
NodeType get_node_type(void* node);

extern const uint32_t NODE_TYPE_OFFSET;

void internal_node_split_and_insert(Table* table, uint32_t parent_page_num,
                                    uint32_t child_page_num);

void print_btree_stats(Pager* pager, uint32_t root_page_num);

uint32_t get_unused_page_num(Pager* pager);

#endif // TABLE_H

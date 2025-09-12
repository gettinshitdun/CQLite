#ifndef TABLE_H
#define TABLE_H

#include <stdint.h>
#include <stdlib.h>

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
#define TABLE_MAX_PAGES 100

typedef struct {
    uint32_t id;
    char     username[COLUMN_USERNAME_SIZE + 1];
    char     email[COLUMN_EMAIL_SIZE + 1];
} Row;

typedef struct {
    int      file_descriptor;
    uint32_t file_length;
    void*    pages[TABLE_MAX_PAGES];
} Pager;

typedef struct {
    uint32_t num_rows;
    Pager*   pager;
} Table;

Table* db_open(const char* filename);
Pager* pager_open(const char* filename);
void   db_close(Table* table);
void   serialize_row(Row* source, void* destination);
void   deserialize_row(void* source, Row* destination);
void*  row_slot(Table* table, uint32_t row_num);
void   print_row(Row* row);

extern const uint32_t TABLE_MAX_ROWS;
void*                 get_page(Pager* pager, uint32_t page_num);

Pager* pager_open(const char* filename);
void   pager_flush(Pager* pager, uint32_t page_num, uint32_t size);

#endif // TABLE_H

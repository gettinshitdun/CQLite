#ifndef STATEMENT_H
#define STATEMENT_H

#include "table.h"
#include "input.h"

typedef enum { META_COMMAND_SUCCESS, META_COMMAND_UNRECOGNIZED_COMMAND } MetaCommandResult;

typedef enum {
    PREPARE_NEGATIVE_ID,
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT,
    PREPARE_STRING_TOO_LONG,
    PREPARE_SYNTAX_ERROR
} PrepareResult;

typedef enum { STATEMENT_INSERT, STATEMENT_SELECT } StatementType;

typedef enum { EXECUTE_TABLE_FULL, EXECUTE_DUPLICATE_KEY, EXECUTE_SUCCESS,EXECUTE_DUPLICATE_TABLE_OR_INDEX } ExecuteResult;

typedef struct {
    StatementType type;
    SchemaRow     schema_row_to_insert;
} Statement;

MetaCommandResult execute_meta_command(InputBuffer* input_buffer, Table* table);
PrepareResult     prepare_statement(InputBuffer* input_buffer, Statement* statement);
ExecuteResult     execute_statement(Statement* statement, Table* table);

#endif // STATEMENT_H

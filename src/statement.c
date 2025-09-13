#include "statement.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

MetaCommandResult execute_meta_command(InputBuffer* input_buffer, Table* table) {
    if (strcmp(input_buffer->buffer, ".exit") == 0) {
        close_input_buffer(input_buffer);
        db_close(table);
        exit(EXIT_SUCCESS);
    } else {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

static PrepareResult prepare_insert(InputBuffer* input_buffer, Statement* statement) {
    statement->type = STATEMENT_INSERT;
    strtok(input_buffer->buffer, " "); // skip "insert"
    char* id_string = strtok(NULL, " ");
    char* username  = strtok(NULL, " ");
    char* email     = strtok(NULL, " ");

    if (id_string == NULL || username == NULL || email == NULL) {
        return PREPARE_SYNTAX_ERROR;
    }

    int id = atoi(id_string);
    if (id < 0) {
        return PREPARE_NEGATIVE_ID;
    }
    if (strlen(username) > COLUMN_USERNAME_SIZE || strlen(email) > COLUMN_EMAIL_SIZE) {
        return PREPARE_STRING_TOO_LONG;
    }

    statement->row_to_insert.id = id;
    strcpy(statement->row_to_insert.username, username);
    strcpy(statement->row_to_insert.email, email);

    return PREPARE_SUCCESS;
}

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement) {
    if (strncmp(input_buffer->buffer, "insert", 6) == 0) {
        return prepare_insert(input_buffer, statement);
    }
    if (strcmp(input_buffer->buffer, "select") == 0) {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    return PREPARE_UNRECOGNIZED_STATEMENT;
}

static ExecuteResult execute_insert(Statement* statement, Table* table) {
    if (table->num_rows >= TABLE_MAX_ROWS) {
        return EXECUTE_TABLE_FULL;
    }
    Row* row_to_insert = &statement->row_to_insert;
    Cursor *cursor = table_end(table);
    serialize_row(row_to_insert, cursor_value(cursor));
    table->num_rows += 1;
    free(cursor);
    return EXECUTE_SUCCESS;
}

static ExecuteResult execute_select(Statement* statement, Table* table) {
    Row row;

    Cursor* cursor = table_start(table);
    (void) statement; // mark as intentionally unused
    while(!(cursor->end_of_table)){
        deserialize_row(cursor_value(cursor),&row);
        print_row(&row);
        cursor_advance(cursor);
    }

    free(cursor);
    return EXECUTE_SUCCESS;
}

ExecuteResult execute_statement(Statement* statement, Table* table) {
    switch (statement->type) {
        case STATEMENT_INSERT:
            return execute_insert(statement, table);
        case STATEMENT_SELECT:
            return execute_select(statement, table);
        default:
            return EXECUTE_SUCCESS;
    }
}

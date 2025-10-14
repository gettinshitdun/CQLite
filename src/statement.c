#include "statement.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

MetaCommandResult execute_meta_command(InputBuffer* input_buffer, Table* table) {
    if (strcmp(input_buffer->buffer, ".exit") == 0) {
        close_input_buffer(input_buffer);
        db_close(table);
        exit(EXIT_SUCCESS);
    } else if (strcmp(input_buffer->buffer, ".btree") == 0) {
        printf("Tree:\n");
        print_tree(table->pager, 0, 0);
        return META_COMMAND_SUCCESS;
    } else if (strcmp(input_buffer->buffer, ".constants") == 0) {
        printf("Constants:\n");
        print_constants();
        return META_COMMAND_SUCCESS;
    } else if (strcmp(input_buffer->buffer, ".printstats") == 0) {
        print_btree_stats(table->pager, table->root_page_num);
        return META_COMMAND_SUCCESS;
    } else {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

static PrepareResult prepare_create_schema(InputBuffer* input_buffer, Statement* statement) {
    statement->type = STATEMENT_CREATE;

    char* buffer = input_buffer->buffer;

    printf("[DEBUG] Input buffer: '%s'\n", buffer);

    // Skip leading spaces
    while (*buffer == ' ') buffer++;

    // Must start with "CREATE"
    if (strncasecmp(buffer, "CREATE", 6) != 0) return PREPARE_SYNTAX_ERROR;
    buffer += 6;
    while (*buffer == ' ') buffer++;

    // Pointer to full SQL = original buffer
    char* sql_str = input_buffer->buffer;
    while (*sql_str == ' ') sql_str++; // trim leading spaces

    // Extract type_str and name_str using temporary buffers (don't modify input_buffer)
    char type_str[MAX_TYPE_LEN];
    char name_str[MAX_NAME_LEN];

    if (sscanf(buffer, "%15s %63s", type_str, name_str) != 2) {
        return PREPARE_SYNTAX_ERROR;
    }

    char* tbl_name_str = (strcasecmp(type_str, "INDEX") == 0) ? name_str : name_str;

    printf("[DEBUG] type_str: '%s'\n", type_str);
    printf("[DEBUG] name_str: '%s'\n", name_str);
    printf("[DEBUG] tbl_name_str: '%s'\n", tbl_name_str);
    printf("[DEBUG] sql_str: '%s'\n", sql_str);

    // Validate type
    if (strcasecmp(type_str, "TABLE") != 0 && strcasecmp(type_str, "INDEX") != 0) {
        printf("[DEBUG] Invalid type: '%s'\n", type_str);
        return PREPARE_SYNTAX_ERROR;
    }

    // Minimal SQL validation
    if (strcasecmp(type_str, "TABLE") == 0) {
        if (strncasecmp(sql_str, "CREATE TABLE", 12) != 0 ||
            strchr(sql_str, '(') == NULL || strchr(sql_str, ')') == NULL) {
            printf("[DEBUG] TABLE SQL validation failed\n");
            return PREPARE_SYNTAX_ERROR;
        }
    } else { // INDEX
        if (strncasecmp(sql_str, "CREATE INDEX", 12) != 0 ||
            !strstr(sql_str, tbl_name_str)) {
            printf("[DEBUG] INDEX SQL validation failed\n");
            return PREPARE_SYNTAX_ERROR;
        }
    }

    // Populate SchemaRow
    SchemaRow* row = &statement->schema_row_to_insert;
    strncpy(row->type, type_str, MAX_TYPE_LEN);
    strncpy(row->name, name_str, MAX_NAME_LEN);
    strncpy(row->tbl_name, tbl_name_str, MAX_TBL_NAME);
    row->root_page = 0;
    strncpy(row->sql, sql_str, MAX_SQL_LEN);

    printf("[DEBUG] SchemaRow populated: type=%s, name=%s, tbl_name=%s, sql=%s\n",
           row->type, row->name, row->tbl_name, row->sql);

    return PREPARE_SUCCESS;
}

static ExecuteResult execute_create(Statement* statement, Table* schema_table) {
    void* node = get_page(schema_table->pager, schema_table->root_page_num);
    uint32_t num_cells = *leaf_node_num_cells(node);

    SchemaRow* row_to_insert = &statement->schema_row_to_insert;
    uint32_t key_to_insert;
    Cursor* cursor;

    for (uint32_t i = 0; i < num_cells; i++) {
        SchemaRow existing_row;
        deserialize_schema_row(leaf_node_value(node, i), &existing_row);

        if (strcmp(existing_row.type, row_to_insert->type) == 0 &&
            strcmp(existing_row.name, row_to_insert->name) == 0) {
            return EXECUTE_DUPLICATE_TABLE_OR_INDEX; // already exists
            }
    }

    if (row_to_insert->root_page == 0) {
        row_to_insert->root_page = get_unused_page_num(schema_table->pager);

        // Initialize the root page as a B-tree leaf
        void* new_node = get_page(schema_table->pager, row_to_insert->root_page);
        initialize_leaf_node(new_node);
    }

    do {
        row_to_insert->rowid = (uint32_t) rand();
        key_to_insert = row_to_insert->rowid;
        cursor = table_find(schema_table, key_to_insert);
    } while (cursor->cell_num < num_cells &&
             *leaf_node_key(node, cursor->cell_num) == key_to_insert);

    leaf_node_insert(cursor, key_to_insert, row_to_insert);
    free(cursor);

    return EXECUTE_SUCCESS;
}

static PrepareResult prepare_insert_schema(InputBuffer* input_buffer, Statement* statement) {
    statement->type = STATEMENT_INSERT;

    strtok(input_buffer->buffer, " "); // skip "insert"

    char* type_str     = strtok(NULL, " ");
    char* name_str     = strtok(NULL, " ");
    char* tbl_name_str = strtok(NULL, " ");
    char* sql_str      = strtok(NULL, "\0"); // rest of line

    if (!type_str || !name_str || !tbl_name_str || !sql_str) {
        return PREPARE_SYNTAX_ERROR;
    }

    if (strlen(type_str) > MAX_TYPE_LEN ||
        strlen(name_str) > MAX_NAME_LEN ||
        strlen(tbl_name_str) > MAX_TBL_NAME ||
        strlen(sql_str) > MAX_SQL_LEN) {
        return PREPARE_STRING_TOO_LONG;
        }

    SchemaRow* row = &statement->schema_row_to_insert;
    strncpy(row->type, type_str, MAX_TYPE_LEN);
    strncpy(row->name, name_str, MAX_NAME_LEN);
    strncpy(row->tbl_name, tbl_name_str, MAX_TBL_NAME);
    row->root_page = 0;  // engine will allocate during execute_insert
    strncpy(row->sql, sql_str, MAX_SQL_LEN);

    return PREPARE_SUCCESS;
}

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement) {
    if (strncmp(input_buffer->buffer, "insert", 6) == 0) {
        return prepare_insert_schema(input_buffer, statement);
    }

    if (strncmp(input_buffer->buffer,"create",6) == 0) {
        return prepare_create_schema(input_buffer, statement);
    }
    if (strcmp(input_buffer->buffer, "select") == 0) {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    return PREPARE_UNRECOGNIZED_STATEMENT;
}

static ExecuteResult execute_insert(Statement* statement, Table* table) {
    void*    node      = get_page(table->pager, table->root_page_num);
    uint32_t num_cells = *leaf_node_num_cells(node);

    SchemaRow* row_to_insert = &statement->schema_row_to_insert;
    uint32_t   key_to_insert; // use rowid as B-tree key

    Cursor* cursor;

    for (uint32_t i = 0; i < num_cells; i++) {
        SchemaRow existing_row;
        deserialize_schema_row(leaf_node_value(node, i), &existing_row);

        if (strcmp(existing_row.type, row_to_insert->type) == 0 &&
            strcmp(existing_row.name, row_to_insert->name) == 0) {
            return EXECUTE_DUPLICATE_TABLE_OR_INDEX; // object already exists
        }
    }

    // allocate root page number
    if (row_to_insert->root_page == 0) {
        row_to_insert->root_page = get_unused_page_num(table->pager);
        void* new_node = get_page(table->pager, row_to_insert->root_page);
        initialize_leaf_node(new_node);
    }

    do {
        row_to_insert->rowid = (uint32_t) rand();
        key_to_insert        = row_to_insert->rowid;
        cursor               = table_find(table, key_to_insert);
    } while (cursor->cell_num < num_cells &&
             *leaf_node_key(node, cursor->cell_num) == key_to_insert);

    leaf_node_insert(cursor, key_to_insert, row_to_insert);
    free(cursor);
    return EXECUTE_SUCCESS;
}

static ExecuteResult execute_select(Statement* statement, Table* table) {
    SchemaRow row;

    Cursor* cursor = table_start(table);
    (void) statement; // mark as intentionally unused
    while (!(cursor->end_of_table)) {
        deserialize_schema_row(cursor_value(cursor), &row);
        print_schema_row(&row);
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
        case STATEMENT_CREATE:
            return execute_create(statement, table);
        default:
            return EXECUTE_SUCCESS;
    }
}

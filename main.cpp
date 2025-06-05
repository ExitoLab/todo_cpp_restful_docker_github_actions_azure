#include "crow.h"
#include <iostream>
#include <sqlite3.h>
#include <string>
#include <vector>

int main() {
    crow::SimpleApp app;

    sqlite3* db;
    int rc = sqlite3_open("todo.db", &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    char* zErrMsg = nullptr;
    rc = sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS todos ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "title TEXT NOT NULL, "
        "completed INTEGER NOT NULL DEFAULT 0);",
        nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        return 1;
    }

    CROW_ROUTE(app, "/health")
    .methods("GET"_method)([]() {
        crow::json::wvalue health_status;
        health_status["status"] = "ok";
        return crow::response{200, health_status};
    });

    CROW_ROUTE(app, "/todos")
    .methods("GET"_method)([&db]() {
        std::vector<crow::json::wvalue> todos;
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, "SELECT id, title, completed FROM todos", -1, &stmt, nullptr) != SQLITE_OK) {
            return crow::response{500, "{\"error\": \"Database error\"}"};
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            crow::json::wvalue todo;
            todo["id"] = sqlite3_column_int(stmt, 0);
            todo["title"] = (const char*)sqlite3_column_text(stmt, 1);
            todo["completed"] = sqlite3_column_int(stmt, 2);
            todos.push_back(todo);
        }
        sqlite3_finalize(stmt);

        return crow::response{200, crow::json::wvalue{{"todos", todos}}};
    });

    CROW_ROUTE(app, "/todos")
    .methods("POST"_method)([&db](const crow::request& req) {
        auto json = crow::json::load(req.body);
        if (!json || !json.has("title")) {
            return crow::response{400, "{\"error\": \"Title is required\"}"};
        }

        std::string title = json["title"].s();
        int completed = 0;

        if (json.has("completed")) {
            try {
                completed = json["completed"].b() ? 1 : 0;
            } catch (const std::runtime_error& e) {
                if (json["completed"].t() == crow::json::type::Number) {
                    completed = json["completed"].i();
                } else {
                    return crow::response{400, "{\"error\": \"Completed must be a boolean or a number\"}"};
                }
            }
        }

        sqlite3_stmt* stmt;
        const char* sql = "INSERT INTO todos (title, completed) VALUES (?, ?)";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return crow::response{500, "{\"error\": \"Database error\"}"};
        }

        sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, completed);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return crow::response{500, "{\"error\": \"Failed to insert todo\"}"};
        }

        sqlite3_finalize(stmt);
        return crow::response{201, "{\"message\": \"Todo added successfully\"}"};
    });

    CROW_ROUTE(app, "/todos/<int>")
    .methods("PUT"_method)([&db](const crow::request& req, int id) {
        auto json = crow::json::load(req.body);
        if (!json || !json.has("completed")) {
            return crow::response{400, "{\"error\": \"Completed status is required\"}"};
        }

        int completed = 0;

        if (json.has("completed")) {
            try {
                completed = json["completed"].b() ? 1 : 0;
            } catch (const std::runtime_error& e) {
                if (json["completed"].t() == crow::json::type::Number) {
                    completed = json["completed"].i();
                } else {
                    return crow::response{400, "{\"error\": \"Completed must be a boolean or a number\"}"};
                }
            }
        }

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, "UPDATE todos SET completed = ? WHERE id = ?", -1, &stmt, nullptr) != SQLITE_OK) {
            return crow::response{500, "{\"error\": \"Database error\"}"};
        }

        sqlite3_bind_int(stmt, 1, completed);
        sqlite3_bind_int(stmt, 2, id);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return crow::response{500, "{\"error\": \"Failed to update todo\"}"};
        }

        sqlite3_finalize(stmt);
        return crow::response{200, "{\"message\": \"Todo updated successfully\"}"};
    });

    CROW_ROUTE(app, "/todos/<int>")
    .methods("DELETE"_method)([&db](int id) {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, "DELETE FROM todos WHERE id = ?", -1, &stmt, nullptr) != SQLITE_OK) {
            return crow::response{500, "{\"error\": \"Database error\"}"};
        }

        sqlite3_bind_int(stmt, 1, id);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return crow::response{500, "{\"error\": \"Failed to delete todo\"}"};
        }

        sqlite3_finalize(stmt);
        return crow::response{200, "{\"message\": \"Todo deleted successfully\"}"};
    });

    CROW_ROUTE(app, "/todos/<int>")
    .methods("GET"_method)([&db](int id) {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, "SELECT id, title, completed FROM todos WHERE id = ?", -1, &stmt, nullptr) != SQLITE_OK) {
            return crow::response{500, "{\"error\": \"Database error\"}"};
        }

        sqlite3_bind_int(stmt, 1, id);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            crow::json::wvalue todo;
            todo["id"] = sqlite3_column_int(stmt, 0);
            todo["title"] = (const char*)sqlite3_column_text(stmt, 1);
            todo["completed"] = sqlite3_column_int(stmt, 2);
            sqlite3_finalize(stmt);
            return crow::response{200, todo};
        } else {
            sqlite3_finalize(stmt);
            return crow::response{404, "{\"error\": \"Todo not found\"}"};
        }
    });

    app.port(8080).bindaddr("0.0.0.0").multithreaded().run();

    sqlite3_close(db);
    return 0;
}
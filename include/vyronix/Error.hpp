#pragma once

#include <string>
#include <stdexcept>
#include <sstream>
#include <iostream>

namespace vyronix {

enum class ErrorType {
    SYNTAX,
    SEMANTIC,
    RUNTIME,
    SYSTEM
};

class VyronixError : public std::runtime_error {
public:
    VyronixError(ErrorType type, const std::string& message, 
                 const std::string& file, size_t line, size_t column)
        : std::runtime_error(message), type_(type), message_(message), 
          file_(file), line_(line), column_(column) {
        
        std::ostringstream oss;
        oss << "[" << typeToString(type) << " ERROR] " << file << ":" << line << ":" << column 
            << ": " << message;
        full_message_ = oss.str();
    }

    const char* what() const noexcept override {
        return full_message_.c_str();
    }

    ErrorType type() const { return type_; }
    const std::string& file() const { return file_; }
    size_t line() const { return line_; }
    size_t column() const { return column_; }

    void report(const std::string& source) const {
        std::cerr << "\033[1;31m[" << typeToString(type_) << " ERROR]\033[0m \033[1m" << file_ << ":" << line_ << ":" << column_ << "\033[0m: " << message_ << "\n";
        
        if (line_ > 0) {
            std::istringstream iss(source);
            std::string line_content;
            for (size_t i = 0; i < line_; ++i) {
                if (!std::getline(iss, line_content)) break;
            }
            
            if (!line_content.empty()) {
                std::cerr << "  " << line_ << " | " << line_content << "\n";
                std::cerr << "    | ";
                for (size_t i = 1; i < column_; ++i) {
                    if (line_content[i-1] == '\t') std::cerr << "\t";
                    else std::cerr << " ";
                }
                std::cerr << "\033[1;32m^\033[0m\n";
            }
        }
    }

private:
    ErrorType type_;
    std::string message_;
    std::string file_;
    size_t line_;
    size_t column_;
    std::string full_message_;

    static std::string typeToString(ErrorType type) {
        switch (type) {
            case ErrorType::SYNTAX:   return "SYNTAX";
            case ErrorType::SEMANTIC: return "SEMANTIC";
            case ErrorType::RUNTIME:  return "RUNTIME";
            case ErrorType::SYSTEM:    return "SYSTEM";
            default:                  return "UNKNOWN";
        }
    }
};

class SyntaxError : public VyronixError {
public:
    SyntaxError(const std::string& message, const std::string& file, size_t line, size_t column)
        : VyronixError(ErrorType::SYNTAX, message, file, line, column) {}
};

class SemanticError : public VyronixError {
public:
    SemanticError(const std::string& message, const std::string& file, size_t line, size_t column)
        : VyronixError(ErrorType::SEMANTIC, message, file, line, column) {}
};

class RuntimeError : public VyronixError {
public:
    RuntimeError(const std::string& message, const std::string& file, size_t line, size_t column)
        : VyronixError(ErrorType::RUNTIME, message, file, line, column) {}
};

class FileReadError : public VyronixError {
public:
    FileReadError(const std::string& message, const std::string& file, size_t line, size_t column)
        : VyronixError(ErrorType::SYSTEM, message, file, line, column) {}
};
} // namespace vyronix

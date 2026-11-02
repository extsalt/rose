#include "Lexer.h"
#include "Parser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

void analyzeSource(const std::string& title, const std::string& sourceCode, bool showTokens = false) {
    std::cout << "=========================================================\n";
    std::cout << " " << title << "\n";
    std::cout << "=========================================================\n";
    std::cout << "[Input Source]:\n" << sourceCode << "\n";

    Lexer lexer(sourceCode);
    std::vector<Token> tokens = lexer.tokenize();

    if (showTokens) {
        std::cout << "\n[Tokens]:\n";
        for (const auto& token : tokens) {
            std::cout << "  line " << token.location.line << ":" << token.location.column
                      << "\t[" << tokenTypeToString(token.type) << "]\t" << token.text << "\n";
        }
    }

    Parser parser(tokens);
    auto ast = parser.parse();

    std::cout << "\n[Syntax Analysis Result]:\n";
    if (parser.hasErrors()) {
        std::cout << "  Status: FAILED (" << parser.getErrors().size() << " syntax error(s))\n";
        for (const auto& err : parser.getErrors()) {
            std::cout << "  --> [Syntax Error] line " << err.location.line << ":" << err.location.column
                      << ": " << err.message;
            if (!err.tokenText.empty()) {
                std::cout << " (at '" << err.tokenText << "')";
            }
            std::cout << "\n";
        }
    } else {
        std::cout << "  Status: SUCCESS (Valid PHP-like Syntax)\n\n";
        std::cout << "[Abstract Syntax Tree (AST)]:\n";
        if (ast) {
            ast->dump(1, std::cout);
        }
    }
    std::cout << "\n";
}

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        std::string arg1 = argv[1];
        if (arg1 == "-e" && argc >= 3) {
            // Inline code
            std::string code = argv[2];
            analyzeSource("Inline Code Analysis", code, true);
            return 0;
        }

        // File input
        std::ifstream file(arg1);
        if (!file.is_open()) {
            std::cerr << "Error: Unable to open file: " << arg1 << "\n";
            return 1;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        analyzeSource("File Analysis: " + arg1, buffer.str(), false);
        return 0;
    }

    // Default demonstration suite
    std::cout << "ROSE - Simple PHP-like Syntax Analyser\n";
    std::cout << "Running test suite...\n\n";

    // Demo 1: Basic variables, arithmetic expressions, and echo
    std::string sample1 = 
        "<?php\n"
        "$a = 10;\n"
        "$b = 20 + 5 * 2;\n"
        "$total = $a + $b;\n"
        "echo \"Total is: \", $total;\n"
        "?>";
    analyzeSource("Test 1: Variable Assignments & Expressions", sample1);

    // Demo 2: Control flow (if-else and while loop)
    std::string sample2 =
        "<?php\n"
        "$x = 5;\n"
        "if ($x > 0) {\n"
        "    $status = \"positive\";\n"
        "} else {\n"
        "    $status = \"non-positive\";\n"
        "}\n"
        "\n"
        "while ($x > 0) {\n"
        "    echo $x;\n"
        "    $x = $x - 1;\n"
        "}\n"
        "?>";
    analyzeSource("Test 2: Control Flow (if-else and while loop)", sample2);

    // Demo 3: Function declaration and call
    std::string sample3 =
        "<?php\n"
        "function add($x, $y) {\n"
        "    return $x + $y;\n"
        "}\n"
        "\n"
        "$res = add(15, 30);\n"
        "echo \"Result: \", $res;\n"
        "?>";
    analyzeSource("Test 3: Function Declaration and Invocation", sample3);

    // Demo 4: Syntax error detection and error recovery
    std::string sample4 =
        "<?php\n"
        "$a = 10\n"        // Missing semicolon
        "$b = 20;\n"
        "if ($b > ) {\n"    // Malformed expression in condition
        "    echo $b;\n"
        "}\n"
        "?>";
    analyzeSource("Test 4: Error Handling & Diagnostics", sample4);

    return 0;
}

#include "../include/context.h"
#include "../include/io.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/result.h"
#include "../include/runner.h"
#include "../include/stdlib.h"
#include "../include/util.h"
#include <stdio.h>
#include <string.h>

/**
 * Runs the given Klein file.
 *
 * # Parameters
 *
 * - `filePath` - The path to the klein file as a null-terminated string.
 *
 * # Returns
 *
 * `NULL`
 *
 * # Errors
 *
 * If an error occurred during tokenization, parsing, or running. For specific documentation
 * on possible errors, see:
 *
 * - `tokenize()` from `lexer.h`
 * - `parse()` from `parser.h`
 * - `evaluate()` from `runner.h`
 */
PRIVATE Result runFile(int numberOfArguments, String arguments[]) {
	if (numberOfArguments < 2) {
		return error("Not enough arguments to run");
	}

	bool usingShorthand = true;
	String filePath = arguments[1];
	if (strcmp(filePath, "run") == 0 && numberOfArguments > 2) {
		filePath = arguments[2];
		usingShorthand = false;
	}

	if (!fileExists(filePath)) {
		if (usingShorthand) {
			fprintf(
				stderr,
				"\n%s The command %s %s requires a file to run.\nIf you meant to run a klein file called \"run\", it doesn't exist.\nRun %s %s for more information.\n\n",
				STYLE("Error:", RED, BOLD),
				STYLE("klein", PURPLE, BOLD),
				STYLE("run", BLUE, BOLD),
				STYLE("klein", PURPLE, BOLD),
				STYLE("help", BLUE, BOLD));
		} else {
			fprintf(
				stderr,
				"\n%s The file \"%s\" doesn't exist.",
				STYLE("Error:", RED, BOLD),
				filePath);
		}
		return error("Invalid command");
	}

	// Not a .klein file
	char* dot = strrchr(filePath, '.');
	if (!dot || strcmp(dot, ".kl")) {
		fprintf(stderr, "\n%s Attempting to run a file that doesn't end with %s. Continue?: ", STYLE("Warning: ", YELLOW, BOLD), COLOR(".kl", CYAN));
		String response = input();
		if (!(strcmp(response, "Y") && strcmp(response, "y") && strcmp(response, "yes") && strcmp(response, "Yes") && strcmp(response, "YES"))) {
			fprintf(stderr, "\n%s\n\n", STYLE("Cancelling.", RED, BOLD));
			exit(1);
		}
		fprintf(stderr, "\n");
	}

	// Read source code
	String rawSourceCode;
	TRY(readFile(filePath, &rawSourceCode), "reading the program's main file");

	// Stdlib
	String sourceCode = malloc(strlen(rawSourceCode) + strlen(STDLIB) + 1);
	ASSERT_NONNULL(sourceCode);
	strcpy(sourceCode, STDLIB);

	// Combine
	strcat(sourceCode, rawSourceCode);
	free(rawSourceCode);

	// Tokenize
	TokenList tokens;
	TRY(tokenize(sourceCode, &tokens), "tokenizing the program's source code");
	free(sourceCode);

	// Context
	CONTEXT = malloc(sizeof(Context));
	TRY(newContext(CONTEXT), "creating the programs context");

	// Parse
	Program program;
	TRY(parse(&tokens, &program), "parsing the program");

	// Run
	TRY(run(program), "running the program");

	// Done
	return OK;
}

/**
 * Wrapper around `main` that returns a `Result` instead of an `int`.
 *
 * More specifically, main wraps *this* function, and converts the `Result` into
 * an exit code. This handles the main logic of the program.
 */
PRIVATE Result mainWrapper(int numberOfArguments, String arguments[]) {
	if (numberOfArguments < 2) {
		printHelp(false);
		return OK;
	};

	if (strcmp(arguments[1], "run") == 0) {
		HANDLE_COMMAND_CONFLICTS("run", return runFile(numberOfArguments, arguments), numberOfArguments, arguments);
	}

	if (strcmp(arguments[1], "help") == 0) {
		HANDLE_COMMAND_CONFLICTS("help", printHelp(false), numberOfArguments, arguments);
		return OK;
	}

	if (fileExists(arguments[1])) {
		return runFile(numberOfArguments, arguments);
	}

	fprintf(
		stderr,
		"\n%s No command or file called \"%s\" exists. Run %s %s for help.\n\n",
		STYLE("Error:", RED, BOLD),
		arguments[1],
		STYLE("klein", PURPLE, BOLD),
		STYLE("help", BLUE, BOLD));
	return OK;
}

/**
 * The program's main entry point. Calls `mainWrapper` and converts the result into an exit
 * code.
 */
int main(int numberOfArguments, String arguments[]) {
	Result attempt = mainWrapper(numberOfArguments, arguments);
	if (isError(attempt)) {
		fprintf(stderr, "\n%s %s\n\n", STYLE("Error:", RED, BOLD), attempt.errorMessage);
		return 1;
	}

	return 0;
}

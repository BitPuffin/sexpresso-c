/* Author: Isak ANdersson 2017 bitpuffin dot com */

/* Dependencies */
/* #include <stddef.h> */

typedef enum {
	SEXPRESSO_SEXP,
	SEXPRESSO_STRING
} sexpresso_value_kind;

struct sexpresso_sexp_s;
typedef struct sexpresso_sexp_s sexpresso_sexp;
struct sexpresso_sexp_s {
	sexpresso_value_kind Kind;
	union {
		struct { sexpresso_sexp* Sexps; size_t Count; } Sexp;
		const char* Str;
	} Value;
};

typedef enum {
	SEXPRESSO_NO_ERROR,
	SEXPRESSO_ERROR_EXCESS_CLOSING_PARENTHESES,
	SEXPRESSO_ERROR_UNEXPECTED_NEWLINE_IN_STRING_LITERAL,
	SEXPRESSO_ERROR_UNTERMINATED_STRING_LITERAL,
	SEXPRESSO_ERROR_UNFINISHED_ESCAPE_SEQUENCE_AT_END_OF_STRING,
	SEXPRESSO_ERROR_INVALID_ESCAPE_CHARACTER,
	SEXPRESSO_ERROR_INCOMPLETE_SEXP
} sexpresso_error_code;

typedef struct {
	sexpresso_error_code Code;
	size_t Line;
	size_t Column;
} sexpresso_error;

sexpresso_sexp sexpressoCreateString(char const* Str);
sexpresso_sexp sexpressoCreateStringUnescaped(char const* cStr);
sexpresso_sexp sexpressoCreateStringUnescapedMove(char const* Str);

void sexpressoAddChild(sexpresso_sexp* Dest, sexpresso_sexp* Child);
void sexpressoAddChildMove(sexpresso_sexp* Dest, sexpresso_sexp* Child);
void sexpressoAddChildString(sexpresso_sexp* Dest, char const* Str);
void sexpressoAddChildStringUnescaped(sexpresso_sexp* Dest, char const* Str);
void sexpressoAddChildStringUnescapedMove(sexpresso_sexp* Dest, char const* Str);

size_t sexpressoChildCount(sexpresso_sexp const* Sexp);

int sexpressoIsString(sexpresso_sexp const* Sexp);
int sexpressoIsSexp(sexpresso_sexp const* Sexp);
int sexpressoIsNil(sexpresso_sexp const* Sexp);

int sexpressoParse(sexpresso_sexp* Dest, const char* Str, sexpresso_error* Err);
const char* sexpressoEscape(const char* Str);

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

sexpresso_sexp* sexpressoParse(const char* Str, const char** Error);
const char* sexpressoEscape(const char* Str);

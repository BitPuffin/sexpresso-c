#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <sexpresso.h>

static void test_empty_string() {
	char const* Str = "";
	assert_true(strlen(Str) == 0);

	sexpresso_sexp Sexp;
	assert_false(sexpressoParse(&Sexp, Str, NULL));

	assert_int_equal(Sexp.Kind, SEXPRESSO_SEXP);
	assert_int_equal(sexpressoChildCount(&Sexp), 0);
	assert_true(sexpressoIsNil(&Sexp));
}

static void test_empty_sexp() {
	char const* Str = "()";

	sexpresso_sexp Sexp;
	assert_false(sexpressoParse(&Sexp, Str, NULL));

	assert_int_equal(Sexp.Kind, SEXPRESSO_SEXP);
	assert_int_equal(sexpressoChildCount(&Sexp), 1);
	assert_int_equal(Sexp.Value.Sexp.Sexps[0].Kind, SEXPRESSO_SEXP);
	assert_int_equal(sexpressoChildCount(&Sexp.Value.Sexp.Sexps[0]), 0);

	char const* Serialized = sexpressoToString(&Sexp);
	assert_string_equal(Serialized, Str);
	free((void*)Serialized);

	sexpressoDestroy(&Sexp);
}

int main(int argc, char** argv) {
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_empty_string),
		cmocka_unit_test(test_empty_sexp),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

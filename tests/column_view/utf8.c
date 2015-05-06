#include <stic.h>

#include <locale.h> /* setlocale() */
#include <string.h>

#include "../../src/utils/utf8.h"
#include "../../src/utils/utils.h"
#include "../../src/column_view.h"

#include "test.h"

static void column_line_print(const void *data, int column_id, const char *buf,
		size_t offset, AlignType align);
static void column1_func(int id, const void *data, size_t buf_len, char *buf);
static void column2_func(int id, const void *data, size_t buf_len, char *buf);
static int locale_works(void);

static const size_t MAX_WIDTH = 20;
static char print_buffer[80 + 1];

SETUP_ONCE()
{
	(void)setlocale(LC_ALL, "");
	if(!locale_works())
	{
		(void)setlocale(LC_ALL, "en_US.utf8");
	}
}

SETUP()
{
	print_next = column_line_print;
	col1_next = column1_func;
	col2_next = column2_func;
}

TEARDOWN()
{
	print_next = NULL;
	col1_next = NULL;
	col2_next = NULL;
}

static void
column_line_print(const void *data, int column_id, const char *buf,
		size_t offset, AlignType align)
{
	strncpy(print_buffer + get_normal_utf8_string_widthn(print_buffer, offset),
			buf, strlen(buf));
}

static void
column1_func(int id, const void *data, size_t buf_len, char *buf)
{
	snprintf(buf, buf_len + 1, "%s", "师从螺丝刀йклмнопрстуфхцчшщьыъэюя");
}

static void
column2_func(int id, const void *data, size_t buf_len, char *buf)
{
	snprintf(buf, buf_len + 1, "%s", "яюэъыьщшчцхфутсрпонмлкйизжёедгв推");
}

static void
perform_test(column_info_t column_infos[], size_t count, size_t max_width)
{
	size_t i;

	columns_t cols = columns_create();
	for(i = 0; i < count; ++i)
	{
		columns_add_column(cols, column_infos[i]);
	}

	memset(print_buffer, '\0', sizeof(print_buffer));
	columns_format_line(cols, NULL, max_width);

	columns_free(cols);
}

TEST(not_truncating_short_utf8_ok, IF(locale_works))
{
	static column_info_t column_infos[1] = {
		{ .column_id = COL1_ID, .full_width = 33UL,    .text_width = 33UL,
		  .align = AT_LEFT,     .sizing = ST_ABSOLUTE, .cropping = CT_TRUNCATE, },
	};
	static const char expected[] = "师从螺丝刀йклмнопрстуфхцчшщьыъэюя       ";

	perform_test(column_infos, 1, 40);

	assert_string_equal(expected, print_buffer);
}

TEST(donot_add_ellipsis_short_utf8_ok, IF(locale_works))
{
	static column_info_t column_infos[1] = {
		{ .column_id = COL1_ID, .full_width = 33UL,    .text_width = 33UL,
		  .align = AT_LEFT,     .sizing = ST_ABSOLUTE, .cropping = CT_ELLIPSIS, },
	};
	static const char expected[] = "师从螺丝刀йклмнопрстуфхцчшщьыъэюя       ";

	perform_test(column_infos, 1, 40);

	assert_string_equal(expected, print_buffer);
}

TEST(truncating_ok, IF(locale_works))
{
	static column_info_t column_infos[2] = {
		{ .column_id = COL1_ID, .full_width = 10UL,    .text_width = 10UL,
		  .align = AT_LEFT,     .sizing = ST_ABSOLUTE, .cropping = CT_TRUNCATE, },
		{ .column_id = COL2_ID, .full_width = 10UL,    .text_width = 10UL,
		  .align = AT_RIGHT,    .sizing = ST_ABSOLUTE, .cropping = CT_TRUNCATE, },
	};
	static const char expected[] = "师从螺丝刀изжёедгв推";

	perform_test(column_infos, 2, MAX_WIDTH);

	assert_string_equal(expected, print_buffer);
}

TEST(none_cropping_allows_for_correct_gaps, IF(locale_works))
{
	static column_info_t column_infos[2] = {
		{ .column_id = COL1_ID, .full_width = 10UL,    .text_width = 10UL,
		  .align = AT_LEFT,     .sizing = ST_AUTO,     .cropping = CT_NONE, },
		{ .column_id = COL2_ID, .full_width = 4UL,     .text_width = 4UL,
		  .align = AT_RIGHT,    .sizing = ST_AUTO,     .cropping = CT_NONE, },
	};
	static const char expected[] = "师从 яюэъыьщшчцхфутсрпонмлкйизжёедгв推";

	perform_test(column_infos, 2, 38);

	assert_string_equal(expected, print_buffer);
}

TEST(add_ellipsis_ok, IF(locale_works))
{
	static column_info_t column_infos[2] = {
		{ .column_id = COL1_ID, .full_width = 10UL,    .text_width = 10UL,
		  .align = AT_LEFT,     .sizing = ST_ABSOLUTE, .cropping = CT_ELLIPSIS, },
		{ .column_id = COL2_ID, .full_width = 10UL,    .text_width = 10UL,
		  .align = AT_RIGHT,    .sizing = ST_ABSOLUTE, .cropping = CT_ELLIPSIS, },
	};
	static const char expected[] = "师从螺... ...ёедгв推";

	perform_test(column_infos, 2, MAX_WIDTH);

	assert_string_equal(expected, print_buffer);
}

TEST(filling, IF(locale_works))
{
	static column_info_t column_infos[1] = {
		{ .column_id = COL1_ID, .full_width = 0UL, .text_width = 0UL,
		  .align = AT_LEFT,     .sizing = ST_AUTO, .cropping = CT_NONE, },
	};
	static const char expected[] = "师从螺丝刀йклмнопрстуфхцчшщьыъэюя       ";

	columns_t cols = columns_create();
	columns_add_column(cols, column_infos[0]);

	memset(print_buffer, '\0', 80);
	columns_format_line(cols, NULL, 40);

	columns_free(cols);

	assert_string_equal(expected, print_buffer);
}

TEST(right_filling, IF(locale_works))
{
	static column_info_t column_infos[1] = {
		{ .column_id = COL2_ID, .full_width = 0UL, .text_width = 0UL,
		  .align = AT_RIGHT,    .sizing = ST_AUTO, .cropping = CT_ELLIPSIS, },
	};
	static const char expected[] = ".";

	columns_t cols = columns_create();
	columns_add_column(cols, column_infos[0]);

	memset(print_buffer, '\0', 80);
	columns_format_line(cols, NULL, 1);

	columns_free(cols);

	assert_string_equal(expected, print_buffer);
}

static int
locale_works(void)
{
	return (vifm_wcwidth(L'丝') == 2);
}

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */

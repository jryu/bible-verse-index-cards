#include <cairo.h>
#include <cairo-pdf.h>
#include <cstdlib>
#include <fcntl.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>
#include <pango/pangocairo.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <stdio.h>

#include "config.pb.h"

config::RendererConfig conf;
auto console = spdlog::stdout_color_mt("console");

bool parse_config()
{
  // Verify that the version of the library that we linked
  // against is compatible with the version of the headers we
  // compiled against.
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  int fd = open("config.txt", O_RDONLY);
  if (fd < 0) {
    console->error("Cannot read config.txt: {}",
        strerror(errno));
    return false;
  }
  google::protobuf::io::FileInputStream fileInput(fd);
  fileInput.SetCloseOnDelete( true );

  if (!google::protobuf::TextFormat::Parse(&fileInput, &conf)) {
    // protobuf prints error message
    return false;
  }
  return true;
}

PangoLayout* init_pango_layout(cairo_t *cr, const std::string& font_family,
    double font_size, PangoWeight weight)
{
  PangoLayout *layout = pango_cairo_create_layout(cr);
  PangoFontDescription *desc =
    pango_font_description_from_string(font_family.c_str());
  pango_font_description_set_weight(desc, weight);
  pango_font_description_set_absolute_size(desc, font_size * PANGO_SCALE);
  pango_layout_set_font_description (layout, desc);
  pango_font_description_free(desc);
  return layout;
}

void get_text_size(cairo_t *cr, const std::string& text,
    int* width, int* height)
{
  PangoLayout *layout = init_pango_layout(cr, conf.text_font_family(),
      conf.text_font_size(), PANGO_WEIGHT_NORMAL);
  pango_layout_set_text(layout, text.c_str(), -1);
  pango_layout_get_size(layout, width, height);
  g_object_unref(layout);
  *width = *width / PANGO_SCALE;
  *height = *height / PANGO_SCALE;
}

std::string trim(const std::string& str)
{
  size_t first = str.find_first_not_of(' ');
  if (std::string::npos == first)
  {
    return str;
  }
  size_t last = str.find_last_not_of(' ');
  return str.substr(first, (last - first + 1));
}

int break_lines(cairo_t *cr, const std::string& input,
    std::vector<std::string>* output)
{
  std::string text = input;
  int total_height = 0;
  for (int i = text.length() - 1; i >= 0; --i) {
    if (i == text.length() - 1 || std::ispunct(text[i])) {
      if (std::ispunct(text[i])) {
        console->info("Punct: {} {}", i, text[i]);
      }
      std::string substr = text.substr(0, i + 1);

      int width, height;
      get_text_size(cr, substr, &width, &height);
      if (width <= conf.card_width() - conf.margin()) {
        // substr fits in one line
        output->push_back(substr);
        total_height += height;

        // Initialize for-loop variables with new text
        text = trim(text.substr(i + 1, text.length() - i - 1));
        i = text.length();
        console->info("text: {}", text);
      }
    }
  }
  if (!text.empty()) {
    output->push_back(text);
    int width, height;
    get_text_size(cr, text, &width, &height);
    total_height += height;

    if (width > conf.card_width()) {
      console->error("Cannot break text without "
          "punctuation: {}", text);
    }
  }
  return total_height;
}

int draw_footer(cairo_t *cr, const std::string& text)
{
  PangoLayout *layout = init_pango_layout(cr, conf.footer_font_family(),
      conf.footer_font_size(), PANGO_WEIGHT_NORMAL);
  pango_layout_set_text(layout, text.c_str(), -1);

  int width, height;
  pango_layout_get_size(layout, &width, &height);
  int margin_bottom = ((double)height / PANGO_SCALE) +
    conf.margin();
  cairo_move_to(cr,
      conf.card_width() - ((double)width / PANGO_SCALE) -
      conf.margin(),
      conf.card_height() - margin_bottom);
  pango_cairo_show_layout(cr, layout);

  g_object_unref(layout);
  return margin_bottom;
}

void render_card(cairo_t *cr, const config::Card& card)
{
  std::vector<std::string> lines;
  int total_height = 0;
  for (const std::string& text : card.text()) {
    total_height += break_lines(cr, text, &lines);
  }

  int margin_bottom = draw_footer(cr, card.footer());

  int y = (conf.card_height() - total_height -
      margin_bottom - conf.margin()) / 2;
  y += conf.margin();

  for (const std::string& text : lines) {
    console->info("drawing: {}", text);
    PangoLayout *layout = init_pango_layout(cr, conf.text_font_family(),
        conf.text_font_size(), PANGO_WEIGHT_NORMAL);
    pango_layout_set_text(layout, text.c_str(), -1);

    int width, height;
    pango_layout_get_size(layout, &width, &height);
    cairo_move_to(cr,
        (conf.card_width() -
         ((double)width / PANGO_SCALE)) / 2,
        y);
    pango_cairo_show_layout(cr, layout);

    g_object_unref(layout);

    y += height / PANGO_SCALE;
  }
}

int main(int argc, char *argv[])
{
  if (!parse_config()) {
    return EXIT_FAILURE;
  }

  cairo_surface_t *surface = NULL;
  surface = cairo_pdf_surface_create("example.pdf",
      conf.card_width(), conf.card_height());
  cairo_t *cr = cairo_create(surface);

  for (const config::Card& card : conf.cards()) {
    render_card(cr, card);
    cairo_show_page(cr);
  }

  cairo_destroy(cr);
  cairo_surface_destroy(surface);

  return EXIT_SUCCESS;
}

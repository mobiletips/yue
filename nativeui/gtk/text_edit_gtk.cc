// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/text_edit.h"

#include <gtk/gtk.h>

#include "nativeui/gtk/undoable_text_buffer.h"
#include "nativeui/gtk/widget_util.h"

namespace nu {

namespace {

void OnTextChange(GtkTextBuffer*, TextEdit* edit) {
  edit->on_text_change.Emit(edit);
}

}  // namespace

TextEdit::TextEdit() {
  GtkWidget* text_view = gtk_text_view_new();
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_CHAR);
  gtk_widget_show(text_view);

  GtkWidget* scroll = gtk_scrolled_window_new(nullptr, nullptr);
  g_object_set_data(G_OBJECT(scroll), "text-view", text_view);
  gtk_container_add(GTK_CONTAINER(scroll), text_view);
  TakeOverView(scroll);

  GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
  TextBufferMakeUndoable(buffer);
  g_signal_connect(buffer, "changed", G_CALLBACK(OnTextChange), this);
}

TextEdit::~TextEdit() {
}

void TextEdit::SetText(const std::string& text) {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  gtk_text_buffer_set_text(buffer, text.c_str(), text.size());
}

std::string TextEdit::GetText() const {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  GtkTextIter start_iter, end_iter;
  gtk_text_buffer_get_start_iter(buffer, &start_iter);
  gtk_text_buffer_get_end_iter(buffer, &end_iter);
  return gtk_text_buffer_get_text(buffer, &start_iter, &end_iter, false);
}

void TextEdit::Redo() {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  TextBufferRedo(buffer);
}

bool TextEdit::CanRedo() const {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  return TextBufferCanRedo(buffer);
}

void TextEdit::Undo() {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  TextBufferUndo(buffer);
}

bool TextEdit::CanUndo() const {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  return TextBufferCanUndo(buffer);
}

void TextEdit::Cut() {
  GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  gtk_text_buffer_cut_clipboard(buffer, clipboard, TRUE);
}

void TextEdit::Copy() {
  GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  gtk_text_buffer_copy_clipboard(buffer, clipboard);
}

void TextEdit::Paste() {
  GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  gtk_text_buffer_paste_clipboard(buffer, clipboard, nullptr, TRUE);
}

void TextEdit::SelectAll() {
  GtkTextView* text_view =
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view"));
  g_signal_emit_by_name(text_view, "select-all", TRUE, nullptr);
}

std::tuple<int, int> TextEdit::GetSelectionRange() const {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  GtkTextIter start_iter, end_iter;
  if (gtk_text_buffer_get_selection_bounds(buffer, &start_iter, &end_iter)) {
    return std::make_tuple(gtk_text_iter_get_offset(&start_iter),
                           gtk_text_iter_get_offset(&end_iter));
  } else {
    return std::make_tuple(-1, -1);
  }
}

void TextEdit::SelectRange(int start, int end) {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  GtkTextIter start_iter, end_iter;
  gtk_text_buffer_get_iter_at_offset(buffer, &start_iter, start);
  gtk_text_buffer_get_iter_at_offset(buffer, &end_iter, end);
  gtk_text_buffer_select_range(buffer, &start_iter, &end_iter);
}

std::string TextEdit::GetTextInRange(int start, int end) const {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  GtkTextIter start_iter, end_iter;
  gtk_text_buffer_get_iter_at_offset(buffer, &start_iter, start);
  gtk_text_buffer_get_iter_at_offset(buffer, &end_iter, end);
  return gtk_text_buffer_get_text(buffer, &start_iter, &end_iter, false);
}

void TextEdit::InsertText(const std::string& text) {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  gtk_text_buffer_insert_at_cursor(buffer, text.c_str(), text.size());
}

void TextEdit::InsertTextAt(const std::string& text, int pos) {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  GtkTextIter iter;
  gtk_text_buffer_get_iter_at_offset(buffer, &iter, pos);
  gtk_text_buffer_insert(buffer, &iter, text.c_str(), text.size());
}

void TextEdit::Delete() {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  gtk_text_buffer_delete_selection(buffer, FALSE, TRUE);
}

void TextEdit::DeleteRange(int start, int end) {
  GtkTextBuffer* buffer = gtk_text_view_get_buffer(
      GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(GetNative()), "text-view")));
  GtkTextIter start_iter, end_iter;
  gtk_text_buffer_get_iter_at_offset(buffer, &start_iter, start);
  gtk_text_buffer_get_iter_at_offset(buffer, &end_iter, end);
  gtk_text_buffer_delete(buffer, &start_iter, &end_iter);
}

}  // namespace nu

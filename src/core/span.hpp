#pragma once

struct Span {
  int file_id;
  int start;
  int end;

  Span(int id, int s, int e) : file_id(id), start(s), end(e) {}
};
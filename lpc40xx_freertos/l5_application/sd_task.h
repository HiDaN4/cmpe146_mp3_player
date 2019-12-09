#pragma once

typedef struct ListNode {
  char *file_name;
  struct ListNode *next;
} list_node_s;

typedef struct sd_list_info {
  int num_of_files;
  list_node_s *list_of_file_names;
} sd_list_files_s;

sd_list_files_s sd__list_mp3_files(void);

void sd__list_cleanup(sd_list_files_s *info);
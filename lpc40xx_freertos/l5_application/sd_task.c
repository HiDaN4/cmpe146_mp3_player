#include "sd_task.h"
#include <stdlib.h>

#include "mp3.h"

#include "ff.h"
#include "sl_string.h"
#include <stdio.h>
#include <string.h>

/// Path on SD Card with mp3 files
/// Note: "/" is root of SD Card
static const char *sd_path_with_mp3_files = "/";

/// Maximum number of files that we can retrieve from the SD card
static const int song_list_max_num = 15;

extern const int song_name_bytes;

/// Get the list of mp3 files on SD card
sd_list_files_s sd__list_mp3_files(void) {
  FRESULT res;
  DIR dir;
  FILINFO fno;

  sd_list_files_s info;
  info.list_of_file_names = NULL;
  info.num_of_files = 0;

  res = f_opendir(&dir, sd_path_with_mp3_files);
  if (res != FR_OK) {
    fprintf(stderr, "Error opening dir at path: %s\n", sd_path_with_mp3_files);
    return info;
  }

  list_node_s *head = NULL;
  list_node_s *prev = NULL;

  int count = 0;
  while (1) {
    res = f_readdir(&dir, &fno);
    if (res != FR_OK || fno.fname[0] == 0)
      break; // exit loop if error reading info or listed all files

    if (count == song_list_max_num)
      break; // stop if reached limit

    if (fno.fattrib & AM_HID)
      continue; // ignore hidden files

    // check if the file is mp3
    if (sl_string__contains_ignore_case(fno.fname, ".MP3")) {
      //   fprintf(stderr, " ** IT IS MP3!\n");

      count += 1;
      // create new node with file name
      head = (list_node_s *)malloc(sizeof(list_node_s));
      head->file_name = (char *)malloc(song_name_bytes * sizeof(char));
      head->next = NULL;
      // first node, assign to struct
      if (info.list_of_file_names == NULL) {
        info.list_of_file_names = head;
      }
      strncpy(head->file_name, fno.fname, song_name_bytes);

      // keep track of last node
      if (prev != NULL) {
        prev->next = head;
      }

      prev = head;
      head = NULL;
    }
  }

  info.num_of_files = count;

  f_closedir(&dir);
  return info;
}

/// Clean up memory used by sd_list_files_s
void sd__list_cleanup(sd_list_files_s *info) {
  if (info->list_of_file_names == NULL)
    return;

  for (list_node_s *node = info->list_of_file_names; node != NULL;) {
    // free memory for file name
    free(node->file_name);
    list_node_s *node_to_delete = node;
    node = node->next;
    // free memory used by node
    free(node_to_delete);
  }

  info->list_of_file_names = NULL;
}
#include "FreeRTOS.h"
#include "cli_handlers.h"
#include "queue.h"
#include <string.h>

extern xQueueHandle Q_songname;
extern int song_name_bytes;

app_cli_status_e cli__task_mp3_play(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
                                    app_cli__print_string_function cli_output) {

  sl_string_t s = user_input_minus_command_name;

  if (sl_string__begins_with_ignore_case(s, "play")) {
    sl_string__erase_first_word(s, ' ');
    char songname[32];
    memset(songname, '\0', sizeof(songname));
    strncpy(songname, s, song_name_bytes - 1);
    xQueueSend(Q_songname, &songname[0], 0);
    sl_string__insert_at(s, 0, "Queued song name: ");
    cli_output(NULL, s);
  } else {
    cli_output(NULL, "Did you mean to say play?\n");
    return APP_CLI_STATUS__HANDLER_FAILURE;
  }

  return APP_CLI_STATUS__SUCCESS;
}
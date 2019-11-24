#include "FreeRTOS.h"
#include "cli_handlers.h"
#include "task.h"

app_cli_status_e cli__task_control(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
                                   app_cli__print_string_function cli_output) {

  sl_string_t s = user_input_minus_command_name;

  if (sl_string__begins_with_ignore_case(s, "suspend")) {
    sl_string__erase_first_word(s, ' ');

    TaskHandle_t task_handle = xTaskGetHandle(s);
    if (task_handle == NULL) {
      sl_string__insert_at(s, 0, "Could not find a task with name: ");
      cli_output(NULL, s);
    } else {
      vTaskSuspend(task_handle);
      sl_string__insert_at(s, 0, "Suspended task with name: ");
      cli_output(NULL, s);
    }
  } else if (sl_string__begins_with_ignore_case(s, "resume")) {
    sl_string__erase_first_word(s, ' ');
    TaskHandle_t task_handle = xTaskGetHandle(s);
    if (task_handle == NULL) {
      sl_string__insert_at(s, 0, "Could not find a task with name: ");
      cli_output(NULL, s);
    } else {
      vTaskResume(task_handle);
      sl_string__insert_at(s, 0, "Resumed task with name: ");
      cli_output(NULL, s);
    }
  } else {
    cli_output(NULL, "Did you mean to say suspend or resume?\n");
  }

  return APP_CLI_STATUS__SUCCESS;
}
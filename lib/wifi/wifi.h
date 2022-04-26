void wifi_init();
bool wifi_is_init();
bool wifi_connected();
bool wifi_attempt_connection();
void wifi_get();
String wifi_post_json(char* json_body);
void wifi_set_url(char*, int);
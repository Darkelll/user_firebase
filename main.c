#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <conio.h>
#include <windows.h>
#include "cJSON.h"

#define ENTER 13
#define TAB 9   
#define BCKSPC 8

#define CACERT_PATH "C:/Users/eldar/Desktop/firebase-c-app"

struct response_buffer {
    char *data;
    size_t size;
};

size_t write_callback(void *ptr, size_t size, size_t nmemb, struct response_buffer *buffer) {
    size_t total_size = size * nmemb;
    char *temp = realloc(buffer->data, buffer->size + total_size + 1);
    if (!temp) {
        fprintf(stderr, "Failed to allocate memory for response.\n");
        return 0;
    }

    buffer->data = temp;
    memcpy(&(buffer->data[buffer->size]), ptr, total_size);
    buffer->size += total_size;
    buffer->data[buffer->size] = '\0'; // terminates null
    return total_size;
}

void send_data_to_firebase(const char *url, const char *data) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        curl_easy_setopt(curl, CURLOPT_CAINFO, CACERT_PATH);  
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);  // SSL verification
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);  // Verify the host

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            printf("\n\n\t\t====\tData sent to Firebase successfully!\t====\n");
        }

        curl_easy_cleanup(curl);
    }
}


int validate_user_from_firebase(const char *url, const char *username, const char *password) {
    CURL *curl;
    CURLcode res;

    struct response_buffer buffer;
    buffer.data = malloc(1); // Initial buffer
    buffer.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&buffer);
        curl_easy_setopt(curl, CURLOPT_CAINFO, CACERT_PATH);  // Add this line
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);  // Ensure SSL verification
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);  // Verify the host

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "\ncurl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            cJSON *json = cJSON_Parse(buffer.data);
            if (!json) {
                fprintf(stderr, "Error parsing JSON: %s\n", cJSON_GetErrorPtr());
            } else {
                cJSON *users = cJSON_GetObjectItemCaseSensitive(json, "users");
                if (users) {
                    cJSON *user;
                    cJSON_ArrayForEach(user, users) {
                        cJSON *username_json = cJSON_GetObjectItemCaseSensitive(user, "username");
                        cJSON *password_json = cJSON_GetObjectItemCaseSensitive(user, "password");

                        if (cJSON_IsString(username_json) && cJSON_IsString(password_json)) {
                            if (strcmp(username, username_json->valuestring) == 0 && strcmp(password, password_json->valuestring) == 0) {
                                cJSON_Delete(json);
                                free(buffer.data);
                                curl_easy_cleanup(curl);
                                curl_global_cleanup();
                                return 1; // User validated
                            }
                        }
                    }
                }
                cJSON_Delete(json);
            }
        }

        curl_easy_cleanup(curl);
        free(buffer.data);
    }

    curl_global_cleanup();
    return 0; // Validation failed
}


struct guest {
    char fullname[50];
    char id[50];
    char password[50];
    char username[50];
};

void takeinput(char ch[50]) {
    fgets(ch, 50, stdin);
    ch[strlen(ch) - 1] = 0;
}

char generateUsername(char fullname[50], char id[50], char username[50]) {
    int i;
    for(i = 0; i < strlen(fullname); i++) {
        if(fullname[i] == ' ') {
            break;
        } else {
            username[i] = fullname[i];
        }
    }
    username[i] = '\0';
    if(strlen(id) >= 7) {
        strncat(username, id + 7, 3);
    }
}

void takepassword(char pwd[50]) {
    int i = 0;
    char ch1;
    while (1) {
        ch1 = getch();
        if (ch1 == ENTER || ch1 == TAB) {
            pwd[i] = '\0';
            break;
        } else if (ch1 == BCKSPC && i > 0) {
            i--;
            printf("\b \b");
        } else {
            pwd[i++] = ch1;
            printf("*");
        }
    }
}

void printWelcome() {
    printf("\n\t\t\t\t=============================================\n");
    printf("\t\t\t\t||       WELCOME TO AUTHORIZATION APP       ||\n");
    printf("\t\t\t\t=============================================\n\n");
}

int main() {
    const char *firebase_url = "https://c2demo-4ded3-default-rtdb.asia-southeast1.firebasedatabase.app/data.json";

    system("color 0d");

    struct guest user;  
    int opt;
    char password2[50] = {0};

    char json_data[200];

    printWelcome();
    printf("\n|| 1. Sign up");
    printf("\n|| 2. Login");
    printf("\n|| 3. Exit\n");
    printf("\nEnter your choice:\t");
    scanf("%d", &opt);
    fgetc(stdin);

    switch(opt) {
        case 1:
            printf("Enter Fullname (Name Surname):\t");
            takeinput(user.fullname);

            while(1) {
                printf("\nEnter Password:\t");
                takepassword(user.password);
                printf("\nConfirm your password:\t");
                takepassword(password2);
                if(strcmp(user.password, password2) == 0) {
                    printf("\nYour password matches!");
                    break;
                } else {
                    Beep(850, 800);
                    printf("\nYour password does not match!\n");
                }
            }

            while(1) {
                printf("\nEnter Your ID:\t");
                takeinput(user.id);
                if(strlen(user.id) == 10) {
                    generateUsername(user.fullname, user.id, user.username);
                    printf("\nYour Username:\t%s\n\n", user.username);
                    snprintf(json_data, sizeof(json_data),
                        "{\"fullname\": \"%s\", \"id\": \"%s\", \"password\": \"%s\", \"username\": \"%s\"}",
                        user.fullname, user.id, user.password, user.username);
                    send_data_to_firebase(firebase_url, json_data);
                    break;
                } else {
                    Beep(850, 800);
                    printf("\nPlease enter a correct ID!\n");
                }
            }
            break;

        case 2:
            {
                char username1[50], pword[50];
                printf("\nEnter your Username:\t");
                takeinput(username1);
                printf("\nEnter your password:\t");
                takepassword(pword);

                if (validate_user_from_firebase(firebase_url, username1, pword)) {
                    printf("\nLogin successful!\n");
                } else {
                    printf("\nInvalid username or password.\n");
                }
            }
            break;

        case 3:
            printf("\n\t\t\t Bye Bye :)");
            return 0;

        default:
            printf("\nIncorrect option!\n");
            break;
    }

    return 0;
}

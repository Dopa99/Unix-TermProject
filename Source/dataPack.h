#include <stdbool.h>

enum UserRole
{
    None = 0,
    Admin, Student, Professor
};

enum Command
{
    NONE = 0,

    USER_LOGIN_REQUEST, USER_LOGOUT_REQUEST,
    LECTURE_LIST_REQUEST, LECTURE_CREATE_REQUEST, LECTURE_REMOVE_REQUEST,
    LECTURE_ENTER_REQUEST, LECTURE_LEAVE_REQUEST, LECTURE_REGISTER_REQUEST, LECTURE_DEREGISTER_REQUEST,
    ATTENDANCE_START_REQUEST, ATTNEDANCE_STOP_REQUEST, ATTNEDANCE_EXTEND_REQUEST, ATTENDANCE_RESULT_REQUEST, ATTENDANCE_CHECK_REQUEST,
    CHAT_ENTER_REQUEST, CHAT_LEAVE_REQUEST, CHAT_USER_LIST_REQUEST, CHAT_SEND_MESSAGE_REQUEST,
    
    USER_LOGIN_RESPONSE, USER_LOGOUT_RESPONSE,
    LECTURE_LIST_RESPONSE, LECTURE_CREATE_RESPONSE, LECTURE_REMOVE_RESPONSE,
    LECTURE_ENTER_RESPONSE, LECTURE_LEAVE_RESPONSE, LECTURE_REGISTER_RESPONSE, LECTURE_DEREGISTER_RESPONSE,
    ATTENDANCE_START_RESPONSE, ATTNEDANCE_STOP_RESPONSE, ATTNEDANCE_EXTEND_RESPONSE, ATTENDANCE_RESULT_RESPONSE, ATTENDANCE_CHECK_RESPONSE,
    CHAT_ENTER_RESPONSE, CHAT_LEAVE_RESPONSE, CHAT_USER_LIST_RESPONSE, CHAT_SEND_MESSAGE_RESPONSE,
};

typedef struct UserInfo_t
{
    enum UserRole role;
    char userName[16];
    char studentID[16];
} UserInfo;

typedef struct DataPack_t
{
    enum Command command;
    bool result;
    char data1[128];
    char data2[128];
    char data3[128];
    char data4[128];
    char message[508 - sizeof(UserInfo)];
    UserInfo userInfo;
} DataPack;

void resetDataPack(DataPack *dataPack);
void buildDataPack(DataPack *targetDataPack, char *data1, char *data2, char *data3, char *data4, char *message);

#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

#define SERVER_PORT 10743
#define SERVER_ADDRESS "192.168.98.128"

enum Command
{
    SampleCommand1, SampleCommand2
};

enum ClientStatus
{
    Login, LectureBrowser, Lobby, Chat
} CurrentClientStatus;

typedef struct DataPack_t
{
    enum Command command;
    char message[508];
} DataPack;

// 서버
bool connectServer();
void async();
void receiveData();
bool sendDataPack(DataPack *dataPack);

bool decomposeDataPack(DataPack *dataPack);
void sendSampleDataPack();

void getUserInput();
void receiveUserCommand();
void updateCommandList();
void updateUserInput();

// 인터페이스
void initiateInterface();
void drawBorder(WINDOW *window, char *windowName);
void onClose();

int ServerSocket;

int Fdmax;
fd_set Master, Reader;

char UserInputGuide[256] = { 0, };
char UserInputBuffer[256] = { 0, };

WINDOW *MessageWindow, *MessageWindowBorder;
WINDOW *CommandWindow, *CommandWindowBorder;
WINDOW *UserInputWindow, *UserInputWindowBorder;

int main()
{
    atexit(onClose);
    initiateInterface();

    if (connectServer())
    {
        CurrentClientStatus = Lobby;
        strncpy(UserInputGuide, "Your Command", sizeof(UserInputGuide));
        updateCommandList();
        updateUserInput();
    }
    else
    {

    }

    async();

    return 0;
}

// TCP/IP 소켓 서버와 연결
// [반환] true: 성공, false: 실패
bool connectServer()
{
    wprintw(MessageWindow, "Try connect to server...\n");
    wrefresh(MessageWindow);

    // TCP/IP 소켓
    ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ServerSocket == -1)
    {
        wprintw(MessageWindow, "socket: %s\n", strerror(errno));
        wrefresh(MessageWindow);
        return false;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(struct sockaddr_in));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);

    // 서버에 연결 요청
    if (connect(ServerSocket, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr_in)) == -1)
    {
        wprintw(MessageWindow, "connect: %s\n", strerror(errno));
        wrefresh(MessageWindow);
        return false;
    }

    wprintw(MessageWindow, "Connected to server\n");
    wrefresh(MessageWindow);
    return true;
}

// 비 동기 입력 대기
void async()
{
    FD_ZERO(&Master);
    FD_ZERO(&Reader);
    FD_SET(0, &Master);
    FD_SET(ServerSocket, &Master);
    Fdmax = ServerSocket;

    while (true)
    {
        Reader = Master;
        if (select(Fdmax + 1, &Reader, NULL, NULL, NULL) == -1)
        {
            wprintw(MessageWindow, "select: %s\n", strerror(errno));
            wrefresh(MessageWindow);
            return;
        }

        for (int fd = 0; fd <= Fdmax; fd++)
        {
            if (FD_ISSET(fd, &Reader) == 0)
                continue;

            if (fd == ServerSocket)
            {
                receiveData();
            }
            else if (fd == 0)   // stdin
            {
                getUserInput();
            }
            else
            {
                // Unknown source
            }
        }
    }
}

// 서버로부터 받은 데이터를 분석
void receiveData()
{
    int readBytes;
    char buffer[sizeof(DataPack)] = { 0, };

    DataPack receivedDataPack;
    DataPack *receivedDataPackPtr = &receivedDataPack;
    memset(&receivedDataPack, 0, sizeof(DataPack));

    readBytes = recv(ServerSocket, buffer, sizeof(buffer), 0);
    switch (readBytes)
    {
        // 예외 처리
        case -1:
            close(ServerSocket);
            FD_CLR(ServerSocket, &Master);
            wprintw(MessageWindow, "recv: %s\n", strerror(errno));
            wrefresh(MessageWindow);
            break;
        
        // 서버와 연결 종료
        case 0:
            close(ServerSocket);
            FD_CLR(ServerSocket, &Master);
            wprintw(MessageWindow, "Disconnected from server\n");
            wrefresh(MessageWindow);
            break;

        // 받은 데이터 유효성 검사
        default:
            if (readBytes == sizeof(DataPack))
            {
                receivedDataPackPtr = (DataPack *)buffer;
                decomposeDataPack(&receivedDataPack);
            }
            else
            {
                wprintw(MessageWindow, "Invalid data received, readBytes: '%d'\n", readBytes);
                wrefresh(MessageWindow);
            }
            break;
    }
}

// 서버에게 전송 받은 DataPack의 Command에 따라 지정된 작업을 수행
// [매개변수] dataPack: 전송 받은 DataPack
// [반환] true: , false: 
bool decomposeDataPack(DataPack *dataPack)
{
    wprintw(MessageWindow, "command: '%d', message: '%s'\n", dataPack->command, dataPack->message);
    wrefresh(MessageWindow);

    switch(dataPack->command)
    {
        case SampleCommand1:
            break;

        case SampleCommand2:
            break;
        
        default:
            break;
    }

    return false;
}

// 서버에게 DataPack을 전송
// [매개변수] dataPack: 전송 할 DataPack
// [반환] true: 전송 성공, false: 전송 실패
bool sendDataPack(DataPack *dataPack)
{
    // wprintw(MessageWindow, "command: '%d', message: '%s'\n", dataPack->command, dataPack->message);
    // wrefresh(MessageWindow);

    int sendBytes;

    sendBytes = send(ServerSocket, (char *)dataPack, sizeof(DataPack), 0);
    switch (sendBytes)
    {
        case -1:
            return false;
        
        case 0:
            return false;
        
        default:
            return true;
    }
}

// DataPack 전송 테스트
void sendSampleDataPack()
{
    DataPack sampleDataPack;
    memset(&sampleDataPack, 0, sizeof(DataPack));

    sampleDataPack.command = SampleCommand2;
    strncpy(sampleDataPack.message, "Definitely not Dopa", sizeof(sampleDataPack.message));

    sendDataPack(&sampleDataPack);

    wprintw(MessageWindow, "Sample DataPack has been sent to the server\n");
    wrefresh(MessageWindow);
}

// 사용자의 키 입력을 읽어옴
void getUserInput()
{
    int userInput;

    userInput = getchar();
    switch (userInput)
    {
        // 엔터
        case 13:
            sendSampleDataPack();
            memset(UserInputBuffer, 0, sizeof(UserInputBuffer));
            break;

        // 백스페이스
        case 127:
            if (strlen(UserInputBuffer) > 0)
            {
                UserInputBuffer[strlen(UserInputBuffer) - 1] = 0;
            }
            break;

        default:
            if (32 <= userInput && userInput <= 126)
            {
                if (strlen(UserInputBuffer) < sizeof(UserInputBuffer) + 1)
                {
                    UserInputBuffer[strlen(UserInputBuffer)] = (char)userInput;
                }
            }
    }

    updateUserInput();
}

// 사용자가 입력한 Command를 처리
void receiveUserCommand()
{
    if (strlen(UserInputBuffer) < 1)
        return;

    switch (CurrentClientStatus)
    {
        case Login:
            break;
        
        case Lobby:
            switch (atoi(UserInputBuffer))
            {
                case 1:
                    break;
                case 2:
                    break;
                case 3:
                    break;
                default:
                    break;
            }
            break;

        case Chat:
            break;

        default:
            break;
    }
}

// CommandWindow의 명령어 목록을 ClientStatus에 따라 업데이트
void updateCommandList()
{
    wclear(CommandWindow);

    switch (CurrentClientStatus)
    {
        case Login:
            break;

        case LectureBrowser:
            wprintw(CommandWindow, "1: 강의 목록 보기\n2: 강의 입장\n\n9: 로그아웃\n0: 프로그램 종료");
        
        case Lobby:
            wprintw(CommandWindow, "1. 공지사항 확인\n2. 출석체크\n3. 강의 멤버 보기\n4. 강의실 나가기\n\n9: 로그아웃\n0: 프로그램 종료");
            break;
            
        case Chat:
            wprintw(CommandWindow, "!quit: 채팅방 나가기\n!user: 채팅방 사용자 보기");
            break;

        default:
            break;
    }

    wrefresh(CommandWindow);
}

// UserInputWindow의 사용자 입력을 업데이트
void updateUserInput()
{
    wclear(UserInputWindow);
    wprintw(UserInputWindow, "%s: %s", UserInputGuide, UserInputBuffer);
    wrefresh(UserInputWindow);
}

// ncurses라이브러리를 이용한 사용자 인터페이스 초기화
void initiateInterface()
{
    setlocale(LC_CTYPE, "ko_KR.utf-8");
    initscr();
    noecho();
    curs_set(FALSE);

    int parentX, parentY;
    getmaxyx(stdscr, parentY, parentX);

    int commandWindowBorderWidth = 22;
    int userInputWindowBorderHeight = 4;

    MessageWindow = newwin(parentY - userInputWindowBorderHeight - 2, parentX - commandWindowBorderWidth - 2, 1, 1);
    MessageWindowBorder = newwin(parentY - userInputWindowBorderHeight, parentX - commandWindowBorderWidth, 0, 0);

    CommandWindow = newwin(parentY - userInputWindowBorderHeight - 2, commandWindowBorderWidth - 2, 1, parentX - commandWindowBorderWidth + 1);
    CommandWindowBorder = newwin(parentY - userInputWindowBorderHeight, commandWindowBorderWidth, 0, parentX - commandWindowBorderWidth);

    UserInputWindow = newwin(userInputWindowBorderHeight - 2, parentX - 2, parentY - userInputWindowBorderHeight + 1, 1);
    UserInputWindowBorder = newwin(userInputWindowBorderHeight, parentX, parentY - userInputWindowBorderHeight, 0);

    scrollok(MessageWindow, TRUE);
    scrollok(CommandWindow, TRUE);

    drawBorder(MessageWindowBorder, "MESSAGE");
    drawBorder(CommandWindowBorder, "COMMAND");
    drawBorder(UserInputWindowBorder, "USER INPUT");

    wrefresh(MessageWindow);
    wrefresh(CommandWindow);
    wrefresh(UserInputWindow);
}

// 윈도우 테두리 그리기
// [매개변수] window: 테두리를 그릴 윈도우, windowName: 상단에 보여줄 윈도우 이름
void drawBorder(WINDOW *window, char *windowName)
{
    int x, y;
    getmaxyx(window, y, x);

    // 테두리 그리기
    mvwprintw(window, 0, 0, "+"); 
    mvwprintw(window, y - 1, 0, "+"); 
    mvwprintw(window, 0, x - 1, "+"); 
    mvwprintw(window, y - 1, x - 1, "+"); 
    for (int i = 1; i < (y - 1); i++)
    {
        mvwprintw(window, i, 0, "|"); 
        mvwprintw(window, i, x - 1, "|"); 
    }
    for (int i = 1; i < (x - 1); i++)
    {
        mvwprintw(window, 0, i, "-"); 
        mvwprintw(window, y - 1, i, "-"); 
    }

    // 윈도우 이름 출력
    mvwprintw(window, 0, 4, windowName); 

    wrefresh(window);
}

// 프로그램 종료시 수행
void onClose()
{
    // ncurses윈도우 종료
    delwin(MessageWindow);
    delwin(MessageWindowBorder);
    delwin(CommandWindow);
    delwin(CommandWindowBorder);
    delwin(UserInputWindow);
    delwin(UserInputWindowBorder);
    endwin();
}

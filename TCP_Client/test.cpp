#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#include <iostream>
#include <thread>
#include <mysql/jdbc.h>
#include <conio.h> 
#include <Windows.h>

using std::cout;
using std::cin;
using std::endl;
using std::string;

#define MAX_SIZE 1024

const string server = "tcp://127.0.0.1:3306";
const string username = "root";
const string password = "07wd2713";

sql::mysql::MySQL_Driver* driver;
sql::Connection* con;
sql::PreparedStatement* pstmt;
sql::ResultSet* result;
sql::Statement* stmt;

SOCKET client_sock;
string my_nick;

void startMenu()
{
    system("cls");
    cout << "\n";
    cout << " "; cout << "*************************************************\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*       *******      *       *       *  *       *\n";
    cout << " "; cout << "*          *        * *      *       * *        *\n";
    cout << " "; cout << "*          *       *****     *       **         *\n";
    cout << " "; cout << "*          *      *     *    *       * *        *\n";
    cout << " "; cout << "*          *     *       *   *****   *  *       *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               < ���� ȭ�� >                   *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               1.�α���                        *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               2. ID ã��                      *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               3. PW ã��                      *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               4. ȸ������                     *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               0. ����                         *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*************************************************\n\n";
}
void login()
{
    system("cls");
    cout << "\n";
    cout << " "; cout << "*************************************************\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*       *******      *       *       *  *       *\n";
    cout << " "; cout << "*          *        * *      *       * *        *\n";
    cout << " "; cout << "*          *       *****     *       **         *\n";
    cout << " "; cout << "*          *      *     *    *       * *        *\n";
    cout << " "; cout << "*          *     *       *   *****   *  *       *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               < �α��� ȭ�� >                 *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*              >> ���̵� �Է�                   *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*              >> ��й�ȣ �Է�                 *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*              >> �Ϸ�� �α���!                *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*              >> ���н� ����ȭ��               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*************************************************\n\n";
}
void searchId()
{
    system("cls");
    cout << "\n";
    cout << " "; cout << "*************************************************\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*       *******      *       *       *  *       *\n";
    cout << " "; cout << "*          *        * *      *       * *        *\n";
    cout << " "; cout << "*          *       *****     *       **         *\n";
    cout << " "; cout << "*          *      *     *    *       * *        *\n";
    cout << " "; cout << "*          *     *       *   *****   *  *       *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               < ���̵� ã�� >                 *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               >> �̸� �Է�                    *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               >> ��ȣ �Է�                    *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               >> �������(8�ڸ�) �Է�         *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               >> �Ϸ�� ���̵� ���           *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               >> ���н� ����ȭ��              *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*************************************************\n\n";
}
void searchPw()
{
    system("cls");
    cout << "\n";
    cout << " "; cout << "*************************************************\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*       *******      *       *       *  *       *\n";
    cout << " "; cout << "*          *        * *      *       * *        *\n";
    cout << " "; cout << "*          *       *****     *       **         *\n";
    cout << " "; cout << "*          *      *     *    *       * *        *\n";
    cout << " "; cout << "*          *     *       *   *****   *  *       *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               < ��й�ȣ ã�� >               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               >> ���̵� �Է�                  *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               >> �̸� �Է�                    *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               >> ��ȭ��ȣ �Է�                *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               >> �������(8�ڸ�) �Է�         *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               >> �Ϸ�� ����ȭ��              *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*************************************************\n\n";
}
void createUser()
{
    system("cls");
    cout << "\n";
    cout << " "; cout << "*************************************************\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*       *******      *       *       *  *       *\n";
    cout << " "; cout << "*          *        * *      *       * *        *\n";
    cout << " "; cout << "*          *       *****     *       **         *\n";
    cout << " "; cout << "*          *      *     *    *       * *        *\n";
    cout << " "; cout << "*          *     *       *   *****   *  *       *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               <  ȸ�� ����  >                 *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               >> ���̵� �Է�                  *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               >> ��й�ȣ �Է�                *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               >> �̸�, ��ȭ��ȣ �Է�          *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               >> �������(8�ڸ�) �Է�         *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               >> �Ϸ�� ����ȭ��              *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*************************************************\n\n";
}
void mainMenu()
{
    Sleep(500);
    system("color 06");
    system("cls");
    cout << "\n";
    cout << " "; cout << "*************************************************\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*       *******      *       *       *  *       *\n";
    cout << " "; cout << "*          *        * *      *       * *        *\n";
    cout << " "; cout << "*          *       *****     *       **         *\n";
    cout << " "; cout << "*          *      *     *    *       * *        *\n";
    cout << " "; cout << "*          *     *       *   *****   *  *       *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*              < ���� ���� : ���� �� >          *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                1. �� ����                     *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                2. ģ��                        *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                3. ��ȭ��                      *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                4. ����                        *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                0. ����                        *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*************************************************\n\n";
}
void myMenu()
{
    system("cls");
    cout << "\n";
    cout << " "; cout << "*************************************************\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*       *******      *       *       *  *       *\n";
    cout << " "; cout << "*          *        * *      *       * *        *\n";
    cout << " "; cout << "*          *       *****     *       **         *\n";
    cout << " "; cout << "*          *      *     *    *       * *        *\n";
    cout << " "; cout << "*          *     *       *   *****   *  *       *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               < �� ���� ���� >                *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*              1. �� ������                     *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*              2. ��� ����/����                *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*              3. BGM ����/����                 *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*              0. �ڷΰ���                      *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*************************************************\n\n";
}
void friends()
{
    system("cls");
    cout << "\n";
    cout << " "; cout << "*************************************************\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*       *******      *       *       *  *       *\n";
    cout << " "; cout << "*          *        * *      *       * *        *\n";
    cout << " "; cout << "*          *       *****     *       **         *\n";
    cout << " "; cout << "*          *      *     *    *       * *        *\n";
    cout << " "; cout << "*          *     *       *   *****   *  *       *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               < �� ģ�� ���� >                *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               1. ģ�� ��� ����               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               2. ģ�� ���� �˻�               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               >> ���� ~ ���� �˻�             *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               0. �ڷΰ���                     *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*************************************************\n\n";
}
void chatting()
{
    system("cls");
    cout << "\n";
    cout << " "; cout << "*************************************************\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*       *******      *       *       *  *       *\n";
    cout << " "; cout << "*          *        * *      *       * *        *\n";
    cout << " "; cout << "*          *       *****     *       **         *\n";
    cout << " "; cout << "*          *      *     *    *       * *        *\n";
    cout << " "; cout << "*          *     *       *   *****   *  *       *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               < ä�� ����/�˻� >              *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               1. ä�ù� ����                  *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               >> /�Ӹ� �̸� �����Է�          *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               >> /���� �Է½� ����            *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               2. ä�� ���� �˻�               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               3. ä�� �Ⱓ �˻�               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               0. �ڷΰ���                     *\n";
    cout << " "; cout << "*************************************************\n\n";
}
void setting()
{
    system("cls");
    cout << "\n";
    cout << " "; cout << "*************************************************\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*       *******      *       *       *  *       *\n";
    cout << " "; cout << "*          *        * *      *       * *        *\n";
    cout << " "; cout << "*          *       *****     *       **         *\n";
    cout << " "; cout << "*          *      *     *    *       * *        *\n";
    cout << " "; cout << "*          *     *       *   *****   *  *       *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               < �� ���� >                     *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               1. ��й�ȣ ����                *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               2. ȸ�� Ż��                    *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*               0. �ڷΰ���                     *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*                                               *\n";
    cout << " "; cout << "*************************************************\n\n";
}
void textColor(int foreground, int background)
{
    int color = foreground + background * 16;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}
int chat_recv() {
    char buf[MAX_SIZE] = { };
    string msg;

    while (1) {
        ZeroMemory(&buf, MAX_SIZE);
        if (recv(client_sock, buf, MAX_SIZE, 0) > 0) {
            textColor(7, 0);
            msg = buf;
            //�г��� : �޽���
            std::stringstream ss(msg);  // ���ڿ��� ��Ʈ��ȭ
            string user;
            ss >> user;
            if (user != my_nick) cout << buf << endl;
        }
        else {
            cout << "Server Off" << endl;
            return -1;
        }
    }
}


class SQL
{
private:
    string id, pw, name, phone, status, birth, song;
public:
    SQL()
    {
        try {
            driver = sql::mysql::get_mysql_driver_instance();
            con = driver->connect(server, username, password);
        }
        catch (sql::SQLException& e) {
            cout << "Could not connect to server. Error message: " << e.what() << endl;
            exit(1);
        }

        con->setSchema("project1");

        // DB �ѱ� ������ ���� ����
        stmt = con->createStatement();
        stmt->execute("set names euckr");
        if (stmt) { delete stmt; stmt = nullptr; }
    }
    int login()
    {
        cout << ">>���̵� : ";
        cin >> id;
        this->id = id;
        cout << ">>��й�ȣ�� �Է����ּ��� : ";
        char ch = ' ';
        while (ch != 13) { // Enter Ű�� ������ �Է� ����
            ch = _getch();
            if (ch == 13) break; // Enter Ű�� ������ �Է� ����
            else if (ch == 8) { // Backspace Ű�� ���
                if (!pw.empty()) { // �Էµ� ���ڰ� ������
                    pw.pop_back(); // ������ ���ڸ� ����
                    cout << "\b \b"; // Ŀ�� ��ġ�� �������� �̵����� ������ ����� ��,
                    // �ٽ� Ŀ�� ��ġ�� ������� �̵���Ŵ
                }
            }
            else {
                pw.push_back(ch);
                cout << '*'; // ��ǥ�� ��ü�Ͽ� ���
            }
        }
        cout << endl;
        pstmt = con->prepareStatement("SELECT id, pw, name FROM user \
            WHERE id=?");
        pstmt->setString(1, id);
        result = pstmt->executeQuery();

        if (result->next())
        { // ���� ����� �ִٸ�
            string db_id = result->getString(1); // �����ͺ��̽����� ������ ID
            string db_pw = result->getString(2); // �����ͺ��̽����� ������ PW

            if (db_id == id && db_pw == pw)
            {
                name = result->getString(3);
                this->name = name;
                cout << endl << "���α��� ���Դϴ�. ��ø� ��ٷ��ּ���." << endl << endl;

                cout << "��" << name << "�� ȯ���մϴ�." << endl;
                return 1;
            }
            else cout << "���ش��ϴ� ������ �����ϴ�." << endl;
        }
        else cout << "���ش��ϴ� ������ �����ϴ�." << endl;
        pw.clear();
    }
    void searchId()
    {
        cout << ">>�̸� : ";
        cin >> name;
        cout << ">>��ȭ��ȣ : ";
        cin >> phone;
        while (true)
        {
            cout << ">>������� : ";
            cin >> birth;
            if (birth.length() != 8) {
                cout << "����������� 8�ڸ��� �Է����ּ���." << endl;
                continue;
            }
            break;
        }

        string year = birth.substr(0, 4);
        string month = birth.substr(4, 2);
        string day = birth.substr(6, 2);
        string DATE = year + "-" + month + "-" + day;

        pstmt = con->prepareStatement("SELECT id, name, phone, birth FROM user WHERE phone=?");
        pstmt->setString(1, phone);
        result = pstmt->executeQuery();

        if (result->next())
        {
            string db_id = result->getString(1);
            string db_name = result->getString(2);
            string db_phone = result->getString(3);
            string db_birth = result->getString(4);

            if (db_name == name && db_phone == phone && db_birth == DATE)
            {
                cout << "��" << name << "���� ���̵�� " << db_id << "�Դϴ�." << endl;
                Sleep(3000);
            }
            else
            {
                cout << "���ش��ϴ� ������ �����ϴ�.!" << endl;
                Sleep(500);
            }
        }
        else {
            cout << "���ش��ϴ� ������ �����ϴ�.?" << endl;
            Sleep(500);
        }
    }
    void searchPw()
    {
        cout << ">>���̵� : ";
        cin >> id;
        cout << ">>�̸� : ";
        cin >> name;
        cout << ">>��ȭ��ȣ : ";
        cin >> phone;
        while (true)
        {
            cout << ">>������� : ";
            cin >> birth;
            if (birth.length() != 8) {
                cout << "����������� 8�ڸ��� �Է����ּ���." << endl;
                continue;
            }
            break;
        }

        string year = birth.substr(0, 4);
        string month = birth.substr(4, 2);
        string day = birth.substr(6, 2);
        string DATE = year + "-" + month + "-" + day;

        pstmt = con->prepareStatement("SELECT id, name, phone, birth FROM user WHERE id=?");
        pstmt->setString(1, id);
        result = pstmt->executeQuery();

        if (result->next())
        {
            string db_id = result->getString(1);
            string db_name = result->getString(2);
            string db_phone = result->getString(3);
            string db_birth = result->getString(4);

            if (db_id == id && db_name == name && db_phone == phone && db_birth == DATE)
            {
                while (1)
                {
                    string new_pw = "";
                    cout << ">>���ο� ��й�ȣ�� �Է����ּ��� : ";
                    char ch = ' ';
                    while (ch != 13) { // Enter Ű�� ������ �Է� ����
                        ch = _getch();
                        if (ch == 13) break; // Enter Ű�� ������ �Է� ����
                        if (ch == 8) { // Backspace Ű�� ���
                            if (!new_pw.empty()) { // �Էµ� ���ڰ� ������
                                new_pw.pop_back(); // ������ ���ڸ� ����
                                cout << "\b \b"; // Ŀ�� ��ġ�� �������� �̵����� ������ ����� ��, �ٽ� Ŀ�� ��ġ�� ������� �̵���Ŵ
                            }
                        }
                        else {
                            new_pw.push_back(ch);
                            cout << '*'; // ��ǥ�� ��ü�Ͽ� ���
                        }
                    }
                    cout << endl;

                    string renew_pw = "";
                    cout << ">>�ٽ� �ѹ� �Է����ּ���. : ";
                    char rech = ' ';
                    while (rech != 13) { // Enter Ű�� ������ �Է� ����
                        rech = _getch();
                        if (rech == 13) break; // Enter Ű�� ������ �Է� ����
                        if (rech == 8) { // Backspace Ű�� ���
                            if (!renew_pw.empty()) { // �Էµ� ���ڰ� ������
                                renew_pw.pop_back(); // ������ ���ڸ� ����
                                cout << "\b \b"; // Ŀ�� ��ġ�� �������� �̵����� ������ ����� ��, �ٽ� Ŀ�� ��ġ�� ������� �̵���Ŵ
                            }
                        }
                        else {
                            renew_pw.push_back(rech);
                            cout << '*'; // ��ǥ�� ��ü�Ͽ� ���
                        }
                    }
                    cout << endl;

                    if (new_pw == renew_pw)
                    {
                        pstmt = con->prepareStatement("UPDATE user SET pw = ? WHERE id = ?");
                        pstmt->setString(1, new_pw);
                        pstmt->setString(2, id);
                        pstmt->executeQuery();
                        printf("�����ο� ��й�ȣ�� ����Ǿ����ϴ�.\n");
                        Sleep(500);
                        break;
                    }
                    else {
                        cout << "����й�ȣ�� �ٸ��ϴ�." << endl;
                        Sleep(500);
                    }
                }
            }
            else {
                cout << "���ش��ϴ� ������ �����ϴ�." << endl;
                Sleep(500);
            }
        }
        else {
            cout << "���ش��ϴ� ������ �����ϴ�." << endl;
            Sleep(500);
        }
    }
    void crateUser()
    {
        while (1)
        {
            cout << ">>���̵� : ";
            cin >> id;
            pstmt = con->prepareStatement("SELECT id FROM user WHERE id=?");
            pstmt->setString(1, id);
            result = pstmt->executeQuery();

            if (result->next())
            {
                string db_id = result->getString(1);
                if (db_id == id) {
                    cout << "���ߺ��� ���̵� �ֽ��ϴ�." << endl;
                    continue;
                }
            }
            else {
                cout << "���ߺ�üũ �Ϸ�." << endl;
                break;
            }
        }
        while (1)
        {
            cout << ">>��й�ȣ�� �Է����ּ��� : ";
            char ch = ' ';
            while (ch != 13) { // Enter Ű�� ������ �Է� ����
                ch = _getch();
                if (ch == 13) break; // Enter Ű�� ������ �Է� ����
                if (ch == 8) { // Backspace Ű�� ���
                    if (!pw.empty()) { // �Էµ� ���ڰ� ������
                        pw.pop_back(); // ������ ���ڸ� ����
                        cout << "\b \b"; // Ŀ�� ��ġ�� �������� �̵����� ������ ����� ��, �ٽ� Ŀ�� ��ġ�� ������� �̵���Ŵ
                    }
                }
                else {
                    pw.push_back(ch);
                    cout << '*'; // ��ǥ�� ��ü�Ͽ� ���
                }
            }
            cout << endl;
            string new_pw = "";
            cout << endl << ">>�ٽ� �ѹ� �Է����ּ���. : ";
            char rech = ' ';
            while (rech != 13) { // Enter Ű�� ������ �Է� ����
                rech = _getch();
                if (rech == 13) break; // Enter Ű�� ������ �Է� ����
                else if (rech == 8) { // Backspace Ű�� ���
                    if (!new_pw.empty()) { // �Էµ� ���ڰ� ������
                        new_pw.pop_back(); // ������ ���ڸ� ����
                        cout << "\b \b"; // Ŀ�� ��ġ�� �������� �̵����� ������ ����� ��, �ٽ� Ŀ�� ��ġ�� ������� �̵���Ŵ
                    }
                }
                else {
                    new_pw.push_back(rech);
                    cout << '*'; // ��ǥ�� ��ü�Ͽ� ���
                }

            }
            cout << endl;

            if (pw == new_pw)
            {
                break;
            }
            else
            {
                cout << "����й�ȣ�� �ٸ��ϴ�." << endl;
                pw.clear();
            }
        }

        cout << ">>�̸� : ";
        cin >> name;
        cout << ">>��ȭ��ȣ : ";
        cin >> phone;
        while (true)
        {
            cout << ">>������� : ";
            cin >> birth;
            if (birth.length() != 8) {
                cout << "����������� 8�ڸ��� �Է����ּ���." << endl;
                continue;
            }
            break;
        }

        string year = birth.substr(0, 4);
        string month = birth.substr(4, 2);
        string day = birth.substr(6, 2);
        string DATE = year + "-" + month + "-" + day;

        pstmt = con->prepareStatement("INSERT INTO user(id,pw, name, phone, birth) VALUE(?,?, ?, ?, ?)");
        pstmt->setString(1, id);
        pstmt->setString(2, pw);
        pstmt->setString(3, name);
        pstmt->setString(4, phone);
        pstmt->setString(5, DATE);
        pstmt->execute();
        cout << "��ȸ�������� �Ϸ�Ǿ����ϴ�." << endl;
        pw.clear();
        Sleep(500);
    }
    void myProfile()
    {
        pstmt = con->prepareStatement("SELECT name, status, song, birth, phone FROM user WHERE id = ?;");
        pstmt->setString(1, id);
        result = pstmt->executeQuery();

        while (result->next())
        {
            cout << "-----------------------------------------" << endl;
            cout << "���̸� : " << result->getString("name") << endl;
            if (result->getString("status") == "")
            {
                cout << "����� : ����" << endl;
            }
            else {
                cout << "����� : " << result->getString("status") << endl;
            }
            if (result->getString("song") == "")
            {
                cout << "���뷡 : ����" << endl;
            }
            else {
                cout << "���뷡 : " << result->getString("song") << endl;
            }
            cout << "������ : " << result->getString("birth") << endl;
            cout << "����ȣ : " << result->getString("phone") << endl;
            cout << "-----------------------------------------" << endl;
        }
    }
    void updateStatus()
    {
        cout << ">>���¸޽��� �Է� : ";
        cin.ignore();
        getline(cin, status);
        pstmt = con->prepareStatement("UPDATE user SET status = ? WHERE id = ?");
        pstmt->setString(1, status);
        pstmt->setString(2, id);
        pstmt->executeQuery();
        printf("��������Ʈ �Ǿ����ϴ�.\n");
    }
    void updateSong()
    {
        cout << ">>�뷡 �Է� : ";
        cin.ignore();
        getline(cin, song);
        pstmt = con->prepareStatement("UPDATE user SET song = ? WHERE id = ?");
        pstmt->setString(1, song);
        pstmt->setString(2, id);
        pstmt->executeQuery();
        printf("��������Ʈ �Ǿ����ϴ�.\n");
    }
    void friends()
    {
        pstmt = con->prepareStatement("SELECT name, status, song, birth, phone FROM user WHERE id != ?;");
        pstmt->setString(1, id);
        result = pstmt->executeQuery();

        while (result->next())
        {
            cout << "-----------------------------" << endl;
            cout << "���̸� : " << result->getString("name") << endl;
            if (result->getString("status") == "")
            {
                cout << "����� : ����" << endl;
            }
            else {
                cout << "����� : " << result->getString("status") << endl;
            }
            if (result->getString("song") == "")
            {
                cout << "���뷡 : ����" << endl;
            }
            else {
                cout << "���뷡 : " << result->getString("song") << endl;
            }
            cout << "������ : " << result->getString("birth") << endl;
            cout << "����ȣ : " << result->getString("phone") << endl;

        }
    }
    void searchBirth()
    {
        string startDay, endDay;
        cout << "���ۿ��� 4�ڸ��� �Է����ּ���.(ex.0201) : ";
        cin >> startDay;// 0303
        cout << "������� 4�ڸ��� �Է����ּ���.(ex.0405) : ";
        cin >> endDay; //0505
        pstmt = con->prepareStatement("SELECT name, birth, phone FROM user \
        WHERE DATE_FORMAT(birth, '%m%d') BETWEEN ? AND ? \
        AND id != ?;");
        pstmt->setString(1, startDay);
        pstmt->setString(2, endDay);
        pstmt->setString(3, id);
        result = pstmt->executeQuery();

        if (!result->next()) {
            cout << "�˻� ����� �����ϴ�." << endl;
        }
        else {
            while (true) {
                cout << "-----------------------------------------------" << endl;
                cout << "���̸� : " << result->getString("name") << endl;
                cout << "������ : " << result->getString("birth") << endl;
                cout << "����ȣ : " << result->getString("phone") << endl;
                if (!result->next()) break; // ���̻� ����� ������ while���� �������ɴϴ�.

            }
        }
    }
    void search_content_Message()
    {
        string content;
        cout << ">>���뿡 ���� �޽��� �˻� : ";
        cin >> content;
        pstmt = con->prepareStatement("SELECT chatname, time, recv FROM chatting\
                               WHERE recv LIKE ?");
        pstmt->setString(1, "%" + content + "%");
        result = pstmt->executeQuery();

        if (!result->next()) {
            cout << "�˻� ����� �����ϴ�." << endl;
        }
        else {
            while (true) {
                string chatname = result->getString(1);
                string time = result->getString(2);
                string recv = result->getString(3);
                cout << "--------------------------------------------------" << endl;
                cout << "��������� : " << chatname << " �������ð� : " << time << endl;
                cout << "��" << recv << endl;
                if (!result->next()) break;
            }
        }
    }
    void search_day_Message()
    {
        string startDay, endDay;
        cout << "���ۿ��� 4�ڸ��� �Է����ּ���.(ex.0201) : ";
        cin >> startDay;// 0303
        cout << "������� 4�ڸ��� �Է����ּ���.(ex.0405) : ";
        cin >> endDay; //0505

        string startMonthDay = "2023" + startDay;
        string endMonthDay = "2023" + endDay;

        pstmt = con->prepareStatement("SELECT chatname, time, recv FROM chatting \
                                   WHERE time BETWEEN ? AND ?");
        pstmt->setString(1, startMonthDay);
        pstmt->setString(2, endMonthDay);
        result = pstmt->executeQuery();

        if (!result->next()) {
            cout << "�˻� ����� �����ϴ�." << endl;
        }
        else {
            while (true) {
                string chatname = result->getString(1);
                string time = result->getString(2);
                string recv = result->getString(3);
                cout << "--------------------------------------------------" << endl;
                cout << "��������� : " << chatname << " �������ð� : " << time << endl;
                cout << "��" << recv << endl;
                if (!result->next()) break;
            }
        }
    }
    void modifyPw()
    {
        cout << ">>���� ��й�ȣ�� �Է����ּ���. : ";
        string existPw = "";
        char ch = ' ';
        while (ch != 13)
        { // Enter Ű�� ������ �Է� ����
            ch = _getch();
            if (ch == 13) break; // Enter Ű�� ������ �Է� ����
            if (ch == 8) { // Backspace Ű�� ���
                if (!existPw.empty()) { // �Էµ� ���ڰ� ������
                    existPw.pop_back(); // ������ ���ڸ� ����
                    cout << "\b \b"; // Ŀ�� ��ġ�� �������� �̵����� ������ ����� ��, �ٽ� Ŀ�� ��ġ�� ������� �̵���Ŵ
                }
            }
            else {
                existPw.push_back(ch);
                cout << '*'; // ��ǥ�� ��ü�Ͽ� ���
            }
        }
        cout << endl;

        pstmt = con->prepareStatement("SELECT pw FROM user WHERE id=?");
        pstmt->setString(1, id);
        result = pstmt->executeQuery();
        if (result->next())
        {
            while (1)
            {
                string new_pw = "";
                cout << ">>���ο� ��й�ȣ�� �Է����ּ��� : ";
                char ch = ' ';
                while (ch != 13) { // Enter Ű�� ������ �Է� ����
                    ch = _getch();
                    if (ch == 13) break; // Enter Ű�� ������ �Է� ����
                    else if (ch == 8) { // Backspace Ű�� ���
                        if (!new_pw.empty()) { // �Էµ� ���ڰ� ������
                            new_pw.pop_back(); // ������ ���ڸ� ����
                            cout << "\b \b"; // Ŀ�� ��ġ�� �������� �̵����� ������ ����� ��, �ٽ� Ŀ�� ��ġ�� ������� �̵���Ŵ
                        }
                    }
                    else {
                        new_pw.push_back(ch);
                        cout << '*'; // ��ǥ�� ��ü�Ͽ� ���
                    }
                }
                cout << endl;

                string renew_pw = "";
                cout << endl << ">>�ٽ� �ѹ� �Է����ּ���. : ";
                char rech = ' ';
                while (rech != 13) { // Enter Ű�� ������ �Է� ����
                    rech = _getch();
                    if (rech == 13) break; // Enter Ű�� ������ �Է� ����
                    else if (rech == 8) { // Backspace Ű�� ���
                        if (!renew_pw.empty()) { // �Էµ� ���ڰ� ������
                            renew_pw.pop_back(); // ������ ���ڸ� ����
                            cout << "\b \b"; // Ŀ�� ��ġ�� �������� �̵����� ������ ����� ��, �ٽ� Ŀ�� ��ġ�� ������� �̵���Ŵ
                        }
                    }
                    else {
                        renew_pw.push_back(rech);
                        cout << '*'; // ��ǥ�� ��ü�Ͽ� ���
                    }

                }
                cout << endl;

                if (new_pw == renew_pw)
                {
                    pstmt = con->prepareStatement("UPDATE user SET pw = ? WHERE id = ?");
                    pstmt->setString(1, new_pw);
                    pstmt->setString(2, id);
                    pstmt->executeQuery();
                    printf("�����ο� ��й�ȣ�� ����Ǿ����ϴ�.\n");
                    break;
                }
                else cout << "����й�ȣ�� �ٸ��ϴ�." << endl;
            }

        }
        else  cout << "����й�ȣ�� �ٸ��ϴ�." << endl;

    }
    int deleteUser()
    {
        cout << ">>���� ��й�ȣ�� �Է����ּ���. : ";
        string existPw = "";
        char ch = ' ';
        while (ch != 13) { // Enter Ű�� ������ �Է� ����
            ch = _getch();
            if (ch == 13) break; // Enter Ű�� ������ �Է� ����
            if (ch == 8) { // Backspace Ű�� ���
                if (!existPw.empty()) { // �Էµ� ���ڰ� ������
                    existPw.pop_back(); // ������ ���ڸ� ����
                    cout << "\b \b"; // Ŀ�� ��ġ�� �������� �̵����� ������ ����� ��, �ٽ� Ŀ�� ��ġ�� ������� �̵���Ŵ
                }
            }
            else {
                existPw.push_back(ch);
                cout << '*'; // ��ǥ�� ��ü�Ͽ� ���
            }
        }
        cout << endl;

        pstmt = con->prepareStatement("SELECT pw FROM user \
            WHERE id=?");
        pstmt->setString(1, id);
        result = pstmt->executeQuery();

        if (result->next())
        { // ���� ����� �ִٸ�
            string db_pw = result->getString(1); // �����ͺ��̽����� ������ PW
            if (db_pw == existPw)
            {
                cout << "���� �����Ͻðڽ��ϱ�? ������ ���Ŀ� �ǵ��� �� �����ϴ�. 1. ����ϱ�, 2. �׸��α� : ";
                char lastCheck;
                cin >> lastCheck;
                if (lastCheck == '1')
                {
                    pstmt = con->prepareStatement("DELETE FROM user WHERE id = ?");
                    pstmt->setString(1, id);
                    result = pstmt->executeQuery();
                    cout << "���׵��� �����߽��ϴ�. �� �̿����ּ���." << endl;
                    return 1;
                }
                else if (lastCheck == '2')
                {
                    cout << "�� �����ϼ̽��ϴ�.." << endl;
                    return 2;
                }
                else cout << "���߸��� �Է��Դϴ�." << endl;

            }
            else cout << "����й�ȣ�� �ٸ��ϴ�." << endl;
        }
        else cout << "����й�ȣ�� �ٸ��ϴ�." << endl;
    }
    string getName()
    {
        string getName = "";
        pstmt = con->prepareStatement("SELECT name FROM user \
            WHERE id=?");
        pstmt->setString(1, id);
        result = pstmt->executeQuery();
        if (result->next())
        {
            getName = result->getString(1);
        }
        return getName;
    }
    void beforeChat()
    {
        pstmt = con->prepareStatement("SELECT chatname, time, recv FROM chatting ORDER BY time DESC LIMIT 5");
        result = pstmt->executeQuery();

        string chatname[5];
        string time[5];
        string recv[5];
        string msg[5];
        int num = 0;
        while (result->next())
        {
            chatname[num] = result->getString(1);
            time[num] = result->getString(2);
            recv[num] = result->getString(3);
            num++;
        }
        for (int i = 4; i >= 0; i--) {
            msg[i] += "--------------------------------------------------";
            msg[i] += "\n��������� : " + chatname[i] + " " + "�������ð� : " + time[i] + "\n";
            msg[i] += "������ : " + recv[i] + "\n";
            msg[i] += "--------------------------------------------------\n";
            cout << msg[i] << endl;
        }
    }
};

int main()
{
    SQL sql;
    bool loginSuccess = false;

    //����ȭ�� ������    
    while (!loginSuccess)
    {
        startMenu(); //���� ȭ��
        char startIn = 0;
        cout << "�� ";
        cin >> startIn;
        switch (startIn)
        {
        case '1': //1. �α���
            login();
            if (sql.login() == 1) {
                loginSuccess = true;
                break;
            }
            continue;
        case '2': //2. ���̵� ã��
            searchId();
            sql.searchId();
            continue;

        case '3': //3. ��й�ȣã��
            searchPw();
            sql.searchPw();
            continue;

        case '4': //4. ȸ������
            createUser();
            sql.crateUser();
            continue;
        case '0': //0. �ý��� ����
            return -1;
        default: //5 ~ 9. ���Է�
            cout << "���߸��� �Է��Դϴ�. �ٽ� �Է����ּ���." << endl;
            continue;
        }

    }

    //����ȭ�� ������
    while (1)
    {
        mainMenu(); //���� ȭ��
        int mainIn = 0;
        cout << "�� ";
        cin >> mainIn;

        //������ ������
        if (mainIn == 1)
        {
            myMenu();
            bool backButton = false;
            while (!backButton)
            {
                char informationIn = 0;
                cout << "�� ";
                cin >> informationIn;
                switch (informationIn)
                {
                case '1':
                    sql.myProfile();
                    break;
                case '2':
                    sql.updateStatus(); //��� ����
                    continue;
                case '3':
                    sql.updateSong(); //�뷡 ����
                    continue;
                case '0':
                    backButton = true;
                    break;
                default:
                    cout << "���߸��� �Է��Դϴ�. �ٽ� �Է����ּ���." << endl;
                    continue;
                }
            }
        }

        //ģ�� ������
        else if (mainIn == 2)
        {
            friends(); //ģ��ȭ��
            bool backButton = false;
            while (!backButton)
            {
                char friendsIn = 0;
                cout << "�� ";
                cin >> friendsIn;
                switch (friendsIn)
                {
                case '1':
                    sql.friends(); //ģ�� ���
                    continue;
                case '2':
                    sql.searchBirth(); //���� �˻�
                    continue;
                case '0':
                    backButton = true;
                    break;
                default:
                    cout << "���߸��� �Է��Դϴ�. �ٽ� �Է����ּ���." << endl;
                    continue;
                }
            }
        }

        //ä�ù� ������
        else if (mainIn == 3)
        {
            chatting(); //ģ��ȭ��
            bool backButton = false;
            int code = 0; // �ʱ�ȭ ����
            while (!backButton)
            {
                char chattingIn = 0;
                cout << "�� ";
                cin >> chattingIn;
                switch (chattingIn)
                {
                case '1':
                    sql.beforeChat();
                    WSADATA wsa;
                    code = WSAStartup(MAKEWORD(2, 2), &wsa); // ������ �ʱ�ȭ ����
                    if (!code)
                    {
                        cout << "< ä�ù濡 �����մϴ�. >" << endl;
                        my_nick = sql.getName();
                        closesocket(client_sock);
                        client_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

                        SOCKADDR_IN client_addr = {};
                        client_addr.sin_family = AF_INET;
                        client_addr.sin_port = htons(7777);
                        InetPton(AF_INET, TEXT("127.0.0.1"), &client_addr.sin_addr);

                        while (1)
                        {
                            if (!connect(client_sock, (SOCKADDR*)&client_addr, sizeof(client_addr))) {
                                cout << "�������� ��Ī�Ͽ� ������ �䱸�� �� ������, ������ �ǽ� �ȴٸ� ��ȭ�� �ߴ����ֽñ� �ٶ��ϴ�." << endl;
                                send(client_sock, my_nick.c_str(), my_nick.length(), 0);
                                break;
                            }
                            cout << "Connecting..." << endl;
                        }
                        std::thread th2(chat_recv);
                        while (1)
                        {
                            string text;
                            std::getline(cin, text);

                            const char* buffer = text.c_str(); // string���� char* Ÿ������ ��ȯ
                            send(client_sock, buffer, strlen(buffer), 0);
                            if (text == "/����")
                            {
                                closesocket(client_sock);
                                backButton = true;
                                break;
                            }
                        }
                        th2.join();
                    }
                    WSACleanup();
                    break;
                case '2':
                    sql.search_content_Message();
                    continue;
                case '3':
                    sql.search_day_Message();
                    continue;
                case '0':
                    backButton = true;
                    break;
                default:
                    cout << "���߸��� �Է��Դϴ�. �ٽ� �Է����ּ���." << endl;
                    continue;
                }

            }
        }

        //������ ����
        else if (mainIn == 4)
        {
            setting(); //����ȭ��
            bool backButton = false;
            while (!backButton)
            {
                char settingIn = 0;
                cout << "�� ";
                cin >> settingIn;
                switch (settingIn)
                {
                case '1':
                    sql.modifyPw(); //��й�ȣ ����
                    continue;
                case '2':
                    if (sql.deleteUser() == 1) return -1;
                    else backButton = true;
                case '0':
                    backButton = true;
                    break;
                default:
                    cout << "���߸��� �Է��Դϴ�. �ٽ� �Է����ּ���." << endl;
                }
            }
        }

        //���� ��ư
        else if (mainIn == 0)
        {
            cout << "���α׷��� �����մϴ�." << endl;
            return 0;
        }

        else cout << "���߸��� �Է��Դϴ�. �ٽ� �Է����ּ���." << endl;
    }
    delete result;
    delete pstmt;
    delete con;
    delete stmt;
    return 0;
}
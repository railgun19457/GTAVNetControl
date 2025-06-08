// version 1.0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <shlobj.h>
#include <conio.h>

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define RULE_NAME_OUT "GTA5-Block-Out"
#define RULE_NAME_IN "GTA5-Block-In"
#define MAX_LIBRARIES 20

// 预定义常见Steam路径
const char *common_steam_paths[] = {
    "C:\\Program Files (x86)\\Steam",
    "C:\\Program Files\\Steam",
    "C:\\steam",
    "C:\\Steam",
    "C:\\STEAM",
    "D:\\steam",
    "D:\\Steam",
    "D:\\STEAM",
    "E:\\steam",
    "E:\\Steam",
    "E:\\STEAM",
    "F:\\steam",
    "F:\\Steam",
    "F:\\STEAM",
    "G:\\steam",
    "G:\\Steam",
    "G:\\STEAM",
    "H:\\steam",
    "H:\\Steam",
    "H:\\STEAM",
    "I:\\steam",
    "I:\\Steam",
    "I:\\STEAM",
    "J:\\steam",
    "J:\\Steam",
    "J:\\STEAM",
    "C:\\SteamLibrary",
    "D:\\SteamLibrary",
    "E:\\SteamLibrary",
    "F:\\SteamLibrary",
    "G:\\SteamLibrary",
    "H:\\SteamLibrary",
    "I:\\SteamLibrary",
    "J:\\SteamLibrary",
    NULL // 结尾标志
};
// 退出程序
void press_any_key_to_exit()
{
    printf("按任意键退出...\n");
    _getch(); // 等待按键，不显示
}
// 判断是否为管理员运行
int is_admin()
{
    BOOL is_admin = FALSE;
    PSID admin_group = NULL;
    SID_IDENTIFIER_AUTHORITY nt_authority = SECURITY_NT_AUTHORITY;

    if (AllocateAndInitializeSid(&nt_authority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                 DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &admin_group))
    {
        CheckTokenMembership(NULL, admin_group, &is_admin);
        FreeSid(admin_group);
    }
    return is_admin;
}

void safe_strcpy(char *dest, const char *src, size_t dest_size)
{
    if (dest_size > 0)
    {
        strncpy(dest, src, dest_size - 1);
        dest[dest_size - 1] = '\0';
    }
}

void trim_newline(char *str)
{
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r'))
    {
        str[--len] = '\0';
    }
}
// 路径标准化
void normalize_path(char *path)
{
    for (char *p = path; *p; p++)
    {
        if (*p == '/')
            *p = '\\';
    }

    size_t len = strlen(path);
    if (len > 0 && path[len - 1] == '\\')
    {
        path[len - 1] = '\0';
    }
}
// 判断文件是否存在
int file_exists(const char *path)

{
    DWORD attrs = GetFileAttributesA(path);
    return (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY));
}
// 路径合法化检查
int validate_gtav_path(const char *path)
{
    if (!path || strlen(path) == 0)
        return 0;

    if (!file_exists(path))
        return 0;

    const char *ext = strrchr(path, '.');
    if (!ext || _stricmp(ext, ".exe") != 0)
        return 0;

    const char *filename = strrchr(path, '\\');
    filename = filename ? filename + 1 : path;

    return (_stricmp(filename, "GTA5_Enhanced.exe") == 0);
}
// 检测GTA安装路径
int find_gtav_path(char *outPath, size_t path_size)
{
    char acfPath[MAX_PATH];
    char installdir[] = "Grand Theft Auto V Enhanced";
    for (int i = 0; common_steam_paths[i] != NULL; i++)
    {
        // 拼接 GTA V 可执行文件路径
        snprintf(outPath, path_size, "%s\\steamapps\\common\\%s\\GTA5_Enhanced.exe", common_steam_paths[i], installdir);
        if (file_exists(outPath))
        {
            return 1;
        }
    }
    return 0;
}
// 手动输入程序路径
int get_manual_path(char *outPath, size_t path_size)
{
    char input[MAX_PATH * 2];

    printf("请输入GTA V可执行文件的完整路径：\n");
    printf("示例：D:\\steam\\steamapps\\common\\Grand Theft Auto V Enhanced\\GTA5_Enhanced.exe\n");
    printf("路径: ");

    if (!fgets(input, sizeof(input), stdin))
    {
        return 0;
    }

    trim_newline(input);

    char *start = input;
    char *end = input + strlen(input) - 1;
    if (*start == '\"' && *end == '\"')
    {
        *end = '\0';
        start++;
    }

    normalize_path(start);
    safe_strcpy(outPath, start, path_size);

    return validate_gtav_path(outPath);
}
// 启动防火墙
void block_gtav(const char *exePath)
{
    char command[2048];

    // 添加出站阻止规则
    snprintf(command, sizeof(command),
             "netsh advfirewall firewall add rule name=\"%s\" "
             "dir=out action=block program=\"%s\" enable=yes >nul 2>&1",
             RULE_NAME_OUT, exePath);
    int result_out = system(command);

    // 添加入站阻止规则
    snprintf(command, sizeof(command),
             "netsh advfirewall firewall add rule name=\"%s\" "
             "dir=in action=block program=\"%s\" enable=yes >nul 2>&1",
             RULE_NAME_IN, exePath);
    int result_in = system(command);

    // 输出结果
    if (result_out == 0 && result_in == 0)
    {
        printf("✅ 已成功阻止 GTA V 的联网访问\n");
    }
    else
    {
        printf("❌ 阻止失败，请确保以管理员权限运行此程序\n");
    }
}
// 关闭防火墙
void unblock_gtav(const char *exePath)
{
    char command[2048];

    // 删除出站规则
    snprintf(command, sizeof(command),
             "netsh advfirewall firewall delete rule name=\"%s\" program=\"%s\">nul 2>&1",
             RULE_NAME_OUT, exePath);
    int result_out = system(command);

    // 删除入站规则
    snprintf(command, sizeof(command),
             "netsh advfirewall firewall delete rule name=\"%s\" program=\"%s\">nul 2>&1",
             RULE_NAME_IN, exePath);
    int result_in = system(command);

    if (result_out == 0 && result_in == 0)
    {
        printf("✅ 已成功恢复 GTA V 的联网访问\n");
    }
    else
    {
        printf("❌ 解除阻止失败，请确保以管理员权限运行此程序\n");
    }
}

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    char gtavPath[MAX_PATH];
    int choice;

    printf("=== GTA V 防火墙控制工具 ===\n\n");

    if (!is_admin())
    {
        printf("⚠️  当前未以管理员权限运行，请右键以管理员身份启动\n\n");
        goto exit;
    }
    else
    {
        printf("✅ 已以管理员权限运行\n");
    }

    if (find_gtav_path(gtavPath, sizeof(gtavPath)))
    {
        printf("✅ 自动检测到 GTA V 路径：\n%s\n\n", gtavPath);
    }
    else
    {
        printf("❌ 未自动检测到 GTA V 安装路径\n");
        printf("请选择操作：\n1. 手动输入路径\n2. 退出程序\n选择: ");

        if (scanf("%d", &choice) != 1 || choice != 1)
        {
            printf("程序退出\n");
            return 1;
        }

        while (getchar() != '\n'); // 清空输入缓冲区

        if (!get_manual_path(gtavPath, sizeof(gtavPath)))
        {
            printf("❌ 输入路径无效，请确保文件存在并为 GTA5_Enhanced.exe\n");
            return 1;
        }

        printf("✅ 已确认路径：%s\n\n", gtavPath);
    }

    while (1)
    {
        printf("\n请选择操作：\n1. 阻止 GTA V 联网\n2. 解除阻止\n3. 退出程序\n选择 (1-3): ");
        if (scanf("%d", &choice) != 1)
        {
            printf("❌ 输入无效，请重新输入\n");
            while (getchar() != '\n');
            continue;
        }

        switch (choice)
        {
        case 1:
            block_gtav(gtavPath);
            break;
        case 2:
            unblock_gtav(gtavPath);
            break;
        case 3:
            unblock_gtav(gtavPath);
            printf("程序退出\n");
            return 0;
        default:
            printf("❌ 无效选择\n");
            break;
        }
    }

exit:
    press_any_key_to_exit();
    return 0;
}

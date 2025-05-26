#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#define FIELD_COUNT 5

// Khai báo prototype cho các hàm
int count_lines(FILE *file_in);
void read_data(FILE *file_in, char ***fields, int count);
void convert_data_raw(FILE *file_out, char ***fields, int count);
void convert_raw_data(FILE *file_out, char ***fields, int count);
void read_raw(FILE *file_in, char ***fields, int count);
uint8_t hexstr_to_uint8(char *hexstr);

uint8_t hexstr_to_uint8(char *hexstr) {
    unsigned int val = 0; 
    sscanf(hexstr, "%2x", &val);
    return (uint8_t)val;
}
void ERROR(int error, int row) {
    FILE *log = fopen("task3.log", "a"); // Mở ở chế độ append
    if (!log) {
        // Thử tạo file mới nếu không mở được
        log = fopen("task3.log", "w");
        if (!log) {
            //printf("Critical error: Cannot create or write to task3.log\n");
            return;
        }
    }
    switch (error) {
        case 1:
            fprintf(log, "Error 01: input file not found or not accessible\n");
            fclose(log);
            exit(0);
            
        case 2:
            fprintf(log, "Error 02: invalid input file format\n");
            fclose(log);
            exit(0);
            
        case 3:
            fprintf(log, "Error 03: invalid command\n");
            fclose(log);
            exit(0);
        case 4:
            fprintf(log, "Error 04: invalid data at line %d\n", row+1);
            break;
        case 5:
            fprintf(log, "Error 05: data packet error at line X\n");
            break;
        case 7:
            fprintf(log, "Error 07: cannot override output file\n");
            break;
        case 8:
            fprintf(log, "Error 08: input file is too large\n");
            break;
    }
    fclose(log);
    
}

// Hàm đếm số dòng trong file
int count_lines(FILE *file_in) {
    int count = 0;
    char ch;
    char last_char = 0;
    
    while ((ch = fgetc(file_in)) != EOF) {
        if (ch == '\n') {
            count++;
        }
        last_char = ch;
    }

    // Nếu file không rỗng và kết thúc không phải là '\n', tăng thêm 1
    if (last_char != '\n' && last_char != 0) {
        count++;
    }

    rewind(file_in);
    return count;
}



// Hàm đọc dữ liệu từ file vào mảng fields
void read_data(FILE *file_in, char ***fields, int count) {
    char line[100];
    int row = 0;
    
    char expected_header[] = "id,time,location,value,condition";
    while (fgets(line, sizeof(line), file_in) && row < count) {
        line[strcspn(line, "\n")] = '\0'; // Xóa ký tự xuống dòng
         char *start = line;
        // Kiểm tra tiêu đề tại dòng 0 (trước khi strtok sửa đổi 'line')
        if (row == 0) {       
            if ( strcmp(start, expected_header) != 0) {
                int error = 2;
                ERROR(error, row); // Ghi lỗi nhưng không thoát 
            }
        }
        //kiem tra dau","
        
        int comma_count = 0;
        for (int i = 0; line[i] != '\0'; i++) {
            if (line[i] == ',') {
                comma_count++;
            }
        }

        if (comma_count > (FIELD_COUNT - 1)) { 
            row++;
            continue; 
        }

        // Bây giờ mới dùng strtok lên 'line' (hoặc line_copy nếu muốn phân tích lại)
        char *token = strtok(line, ","); 
        int col = 0;
        // Đọc các trường vào mảng fields
        while (token && col < FIELD_COUNT) {
            fields[row][col] = strdup(token);
            token = strtok(NULL, ",");
            col++;
        }
        row++;
    }
}

//======================================CHECK_ERROR_DATA===========================================
//ID_LOCATION
int check_id_location(const char *str, int row) {
    // Bỏ qua khoảng trắng đầu/cuối nếu có
    while (isspace((unsigned char)*str)) str++;

    if (*str == '\0') {
        // Chuỗi rỗng sau khi loại khoảng trắng
        
        return 0;
    }

    // Kiểm tra từng ký tự có phải số không
    for (int i = 0; str[i]; i++) {
        if (!isdigit((unsigned char)str[i])) {
            
            return 0;
        }
    }

    // Chuyển thành số
    long value = strtol(str, NULL, 10);
    if (value > 255) {
        // Quá lớn với 1 byte
        
        return 0;
    }

    return 1; // hợp lệ
}
//--------------------------------------------------------------------------------------------
//TIME
int check_datetime(const char *datetime) {
    if (strlen(datetime) != 19) return 0;

    // Kiểm tra định dạng cố định
    if (datetime[4] != ':' || datetime[7] != ':' || datetime[10] != ' ' ||
        datetime[13] != ':' || datetime[16] != ':') {
        return 0;
    }

    // Kiểm tra tất cả các ký tự khác là số
    for (int i = 0; i < 19; i++) {
        if (i == 4 || i == 7 || i == 10 || i == 13 || i == 16) continue;
        if (!isdigit((unsigned char)datetime[i])) return 0;
    }

    int year, month, day, hour, minute, second;
    // Đọc trực tiếp vào các biến
    if (sscanf(datetime, "%4d:%2d:%2d %2d:%2d:%2d",
               &year, &month, &day, &hour, &minute, &second) != 6) {
        return 0;
    }

    if (month < 1 || month > 12) return 0;
    if (day < 1 || day > 31) return 0;
    if (hour < 0 || hour > 23) return 0;
    if (minute < 0 || minute > 59) return 0;
    if (second < 0 || second > 59) return 0;

    int days_in_month[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    // Kiểm tra năm nhuận
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        days_in_month[1] = 29;
    }

    if (day > days_in_month[month - 1]) return 0;

    return 1; // hợp lệ
}

//----------------------------------------------------------------------------------------
int check_float(const char *str) {
    if (str == NULL || *str == '\0') return 0;

    char *endptr;
    double val = strtod(str, &endptr);

    // Kiểm tra: phải có ít nhất 1 ký tự được chuyển thành số
    if (endptr == str) return 0;

    // Kiểm tra không có ký tự lạ ở phần còn lại
    while (*endptr) {
        if (!isspace((unsigned char)*endptr)) return 0;
        endptr++;
    }

    // Nếu tới đây là hợp lệ
    return 1;
}



//========================================================================================
// Hàm chuyển đổi dữ liệu và ghi ra file
// Hàm kiểm tra giá trị Lux (float)
int check_lux(const char *str, int row) {
    if (str == NULL || *str == '\0') {
        return 0;
    }

    char *endptr;
    double val = strtod(str, &endptr);

    // Kiểm tra: phải có ít nhất 1 ký tự được chuyển thành số
    if (endptr == str) {
        return 0;
    }

    // Kiểm tra không có ký tự lạ ở phần còn lại
    while (*endptr) {
        if (!isspace((unsigned char)*endptr)) {
            return 0;
        }
        endptr++;
    }

    // Kiểm tra giá trị hợp lệ (ví dụ: không âm, không quá lớn)
    if (val < 0.1 || val > 100000) { // Giả sử giới hạn Lux
        return 0;
    }

    return 1; // Hợp lệ
}
int data_duplicate(char ***fields, int current_line_idx) {
    for (int n = 0; n < current_line_idx; n++) {
        for (int j = 0; j < FIELD_COUNT; j++) {
            if (fields[current_line_idx][j] == NULL || fields[n][j] == NULL ||
                strcmp(fields[current_line_idx][j], fields[n][j]) != 0) {
                goto next_line_n;
            }
        }
        return n + 2; 
        next_line_n:;
    }
    return -1;
}
// Hàm chuyển đổi dữ liệu và ghi ra file
void convert_data_raw(FILE *file_out, char ***fields, int count) {

    for (int i = 1; i <= count - 1; i++) { // Sửa i < count thành i <= count - 1 để không bỏ qua dòng cuối
        int valid = 1;  // Giả sử dòng hợp lệ
        for (int j = 0; j < FIELD_COUNT; j++) {
            if (!fields[i][j] || strcmp(fields[i][j], "") == 0) {
                int error = 4;
                ERROR(error, i);
                valid = 0;  // Dòng không hợp lệ
                break;      // Không cần kiểm tra thêm
            }
        }
        if (!valid) continue;  // Bỏ qua dòng lỗi
      

        int duplicate_line_num = data_duplicate(fields, i); 
        if (duplicate_line_num != -1) { // Nếu tìm thấy trùng lặp
            FILE *log = fopen("task3.log", "a"); // Mở file log ở chế độ append
            if (log) { // Đảm bảo mở file thành công
                fprintf(log, "Error 06: data at line %d and %d are duplicated\n", duplicate_line_num, i);
                fclose(log);
            }
            continue; // Bỏ qua dòng bị trùng lặp này
        }
        uint8_t data_for_checksum[15];
        int data_index = 0;

        // ID (Cột 0)
        if (!check_id_location(fields[i][0], i)) {
            int error = 4;
            ERROR(error, i);
            continue; // Bỏ qua dòng lỗi
        }
        unsigned int id_val;
        sscanf(fields[i][0], "%u", &id_val);
        data_for_checksum[data_index++] = (uint8_t)id_val;

        // Location (Cột 2)
        if (!check_id_location(fields[i][2], i)) {
            int error = 4;
            ERROR(error, i);
            continue; // Bỏ qua dòng lỗi
        }
        unsigned int location_val;
        sscanf(fields[i][2], "%u", &location_val);
        data_for_checksum[data_index++] = (uint8_t)location_val;

        // Time (Cột 1: YYYY:MM:DD HH:MM:SS, timestamp - 4 bytes, big-endian)
        if (!check_datetime(fields[i][1])) {
            int error = 4;
            ERROR(error, i);
            continue; // Bỏ qua dòng lỗi
        }
        struct tm t = {0};
        sscanf(fields[i][1], "%d:%d:%d %d:%d:%d",
               &t.tm_year, &t.tm_mon, &t.tm_mday,
               &t.tm_hour, &t.tm_min, &t.tm_sec);
        t.tm_year -= 1900; // Chuyển về năm kể từ 1900
        t.tm_mon -= 1;     // Tháng từ 0-11
        time_t timestamp = mktime(&t);
       
        data_for_checksum[data_index++] = (timestamp >> 24) & 0xFF;
        data_for_checksum[data_index++] = (timestamp >> 16) & 0xFF;
        data_for_checksum[data_index++] = (timestamp >> 8) & 0xFF;
        data_for_checksum[data_index++] = timestamp & 0xFF;

        // Lux (Cột 3: Value, float - 4 bytes, big-endian)
        if (!check_lux(fields[i][3], i)) {
            int error = 4;
            ERROR(error, i);
            continue; // Bỏ qua dòng lỗi
        }
        float value = atof(fields[i][3]);
        unsigned char *float_bytes = (unsigned char *)&value;
        int n = 1;
        if (*(char *)&n == 1) { // Little-endian
            data_for_checksum[data_index++] = float_bytes[3];
            data_for_checksum[data_index++] = float_bytes[2];
            data_for_checksum[data_index++] = float_bytes[1];
            data_for_checksum[data_index++] = float_bytes[0];
        } else { // Big-endian
            data_for_checksum[data_index++] = float_bytes[0];
            data_for_checksum[data_index++] = float_bytes[1];
            data_for_checksum[data_index++] = float_bytes[2];
            data_for_checksum[data_index++] = float_bytes[3];
        }

        // Condition (Cột 4)
        uint8_t condition_val = 0;
        if (strcmp(fields[i][4], "NA") == 0) {
            condition_val = 0;
        } else if (strcmp(fields[i][4], "dark") == 0) {
            condition_val = 1;
        } else if (strcmp(fields[i][4], "bright") == 0) {
            condition_val = 3;
        } else if (strcmp(fields[i][4], "good") == 0) {
            condition_val = 2;
        } else {
            int error = 4;
            ERROR(error, i);
            continue; // Bỏ qua dòng lỗi
        }
        data_for_checksum[data_index++] = condition_val;

        // Tính checksum
        uint8_t sum = 0;
        sum = ((timestamp >> 24) & 0xFF) + ((timestamp >> 16) & 0xFF) + ((timestamp >> 8) & 0xFF) + (timestamp & 0xFF);
        sum += (uint8_t)id_val;
        sum += (uint8_t)location_val;
        sum += condition_val;
        sum += 0x0F;
        sum += float_bytes[3];
        sum += float_bytes[2];
        sum += float_bytes[1];
        sum += float_bytes[0];
        uint8_t two_complement = (~sum + 1);

        // Ghi dữ liệu ra file
        fprintf(file_out, "0E 0F %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X FE\n",
                id_val,
                location_val,
                (timestamp >> 24) & 0xFF,
                (timestamp >> 16) & 0xFF,
                (timestamp >> 8) & 0xFF,
                timestamp & 0xFF,
                float_bytes[3], float_bytes[2], float_bytes[1], float_bytes[0],
                condition_val,
                two_complement);
    }
}
//===================================================================================================
void read_raw(FILE *file_in, char ***fields, int count) {
    char line[100];
    int row = 0;

    while (fgets(line, sizeof(line), file_in) && row < count) {
        line[strcspn(line, "\n")] = '\0';

        char *token = strtok(line, " ");
        int col = 0;

        while (token && col < 15) {
            // Xóa khoảng trắng đầu chuỗi
            while (isspace((unsigned char)*token)) token++;

            // Kiểm tra độ dài token (phải là 2 ký tự hex)
            if (strlen(token) != 2 || !isxdigit((unsigned char)token[0]) || !isxdigit((unsigned char)token[1])) {
                int error = 4; // Lỗi dữ liệu tại dòng
                ERROR(error, row);
                row++;
                break;
            }

            fields[row][col] = strdup(token);
            if (!fields[row][col]) { // Kiểm tra lỗi cấp phát bộ nhớ
                int error = 4;
                ERROR(error, row);
                row++;
                break;
            }

            token = strtok(NULL, " ");
            col++;
        }

        // Kiểm tra số cột (phải đúng 15 cột)
        if (col != 15) {
            int error = 4; // Lỗi dữ liệu tại dòng
            ERROR(error, row);
            for (int j = 0; j < col; j++) {
                free(fields[row][j]);
                fields[row][j] = NULL;
            }
            row++;
            continue;
        }

        row++;
    }
}
//================================CHECK_VALID_RAW========================================================
#define PACKET_SIZE 15
#define START_BYTE 0x0E
#define STOP_BYTE 0xFE

// Hàm tính checksum theo mã bù 2
uint8_t calculate_checksum_2comp(const unsigned char *fields) {
    uint16_t sum = 0;
        for (int i = 1; i <= 12; i++) {
        sum += fields[i];
    }
    return (uint8_t)((~sum + 1) & 0xFF);
}

int validate_packet_fields(unsigned char **fields, int i) {
    // Kiểm tra byte đầu và cuối
    if (fields[i][0] != START_BYTE || fields[i][14] != STOP_BYTE) {
        return 0;
    }

    // Kiểm tra độ dài packet
    if (fields[i][1] != PACKET_SIZE) {
        return 0;
    }

    // Kiểm tra checksum
    uint8_t expected_checksum = fields[i][13];
    uint8_t actual_checksum = calculate_checksum_2comp(fields[i]);
    if (expected_checksum != actual_checksum) {
        return 0;
    }

    return 1; // Hợp lệ
}
// check trung lap
int is_duplicate(unsigned char **fields, int current_line) {
    for (int X = 0; X < current_line; X++) {  
        if (memcmp(fields[current_line], fields[X], 15 * sizeof(unsigned char)) == 0) { // So sánh 15 byte
            return X;  // Là dòng trùng lặp
        }
    }
    return -1; // Không trùng
}
void convert_raw_data(FILE *file_out, char ***fields, int count) {
    unsigned char **raw_fields = (unsigned char **)malloc(count * sizeof(unsigned char *));
    for (int i = 0; i < count; i++) {
        raw_fields[i] = (unsigned char *)malloc(15 * sizeof(unsigned char));
        for (int j = 0; j < 15; j++) {
            raw_fields[i][j] = (unsigned char)strtol(fields[i][j], NULL, 16);
        }
    }

    for (int i = 0; i < count; i++) { 
        // Kiểm tra tính hợp lệ của packet
        if (!validate_packet_fields(raw_fields, i)) {
            int error = 5;
            ERROR(error, i);
            continue;
        }

        // Kiểm tra trùng lặp
        int duplicate_line = is_duplicate(raw_fields, i);
        if (duplicate_line != -1) { // Nếu trùng lặp
            FILE *log = fopen("task3.log", "a"); // Mở file log ở chế độ append
            fprintf(log, "Error 06: data at line %d and %d are duplicated\n", duplicate_line, i);
            fclose(log);
            continue;
        }

        int id = (int)strtol(fields[i][2], NULL, 16);
        int location = (int)strtol(fields[i][3], NULL, 16);
        int condition = (int)strtol(fields[i][12], NULL, 16);

        // Ghép timestamp theo thứ tự big-endian: byte thấp đến cao
        char time_hex[9] = ""; // 8 ký tự hex + \0
        strcat(time_hex, fields[i][4]); // Byte thấp nhất
        strcat(time_hex, fields[i][5]);
        strcat(time_hex, fields[i][6]);
        strcat(time_hex, fields[i][7]); // Byte cao nhất
        time_t timestamp = (time_t)strtoul(time_hex, NULL, 16);
        struct tm *tm_info = localtime(&timestamp); // Sửa bằng cách truyền địa chỉ
        if (tm_info == NULL) {
            int error = 4;
            ERROR(error, i);
            continue;
        }
        char time[30];
        strftime(time, sizeof(time), "%Y:%m:%d %H:%M:%S", tm_info);
        // Chuyển đổi Lux (big-endian)
        unsigned char float_bytes[4];
        sscanf(fields[i][8], "%2hhx", &float_bytes[3]);  // Byte thấp nhất
        sscanf(fields[i][9], "%2hhx", &float_bytes[2]);
        sscanf(fields[i][10], "%2hhx", &float_bytes[1]);
        sscanf(fields[i][11], "%2hhx", &float_bytes[0]); // Byte cao nhất

        float value;
        memcpy(&value, float_bytes, sizeof(float));

        char condition_char[10];
        switch (condition) {
            case 0:
                strcpy(condition_char, "NA");
                break;
            case 1:
                strcpy(condition_char, "dark");
                break;
            case 2:
                strcpy(condition_char, "good");
                break;
            case 3:
                strcpy(condition_char, "bright");
                break;
            default:
                strcpy(condition_char, "unknown");
                break;
        }

        fprintf(file_out, "%d,", id);
        fprintf(file_out, "%s,", time);
        fprintf(file_out, "%d,", location);
        fprintf(file_out, "%.2f,", value);
        fprintf(file_out, "%s\n", condition_char);
    }

    // Giải phóng bộ nhớ raw_fields
    for (int i = 0; i < count; i++) {
        free(raw_fields[i]);
    }
    free(raw_fields);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        int error = 3, row = 0;
        ERROR(error, row);
    }

    char input_file[256], output_file[256];
    strcpy(input_file, argv[1]);
    strcpy(output_file, argv[2]);

    FILE *file_in = fopen(input_file, "r");
    if (!file_in) {
        int error = 1, row = 0;
        ERROR(error, row);
    }

    FILE *file_out = fopen(output_file, "w");
    if (!file_out) {
        fclose(file_in);
        int error = 7, row = 0;
        ERROR(error, row);
    }

    int count = count_lines(file_in);
    if (count == 0) {
        int error = 2, row = 0;
        ERROR(error, row);
    }

    const char *ext = strrchr(input_file, '.');
    int raw = -1;
    if (ext != NULL) {
        if (strcmp(ext, ".dat") == 0) {
            raw = 1;
        } else if (strcmp(ext, ".csv") == 0) {
            raw = 0;
        } else {
            int error = 2, row = 0;
            ERROR(error, row);
        }
    }

    FILE *log = NULL;
    switch (raw) {
        case 1:
            {
                char ***fields = (char ***)malloc(count * sizeof(char **));
                for (int i = 0; i < count; i++) {
                    fields[i] = (char **)malloc(15 * sizeof(char *));
                    for (int j = 0; j < 15; j++) {
                        fields[i][j] = NULL;
                    }
                }
                read_raw(file_in, fields, count);
                convert_raw_data(file_out, fields, count);
                for (int i = 0; i < count; i++) {
                    for (int j = 0; j < 15; j++) {
                        free(fields[i][j]);
                    }
                    free(fields[i]);
                }
                free(fields);
            }
            break;
        case 0:
            {
                char ***fields = (char ***)malloc(count * sizeof(char **));
                for (int i = 0; i < count; i++) {
                    fields[i] = (char **)malloc(FIELD_COUNT * sizeof(char *));
                    for (int j = 0; j < FIELD_COUNT; j++) {
                        fields[i][j] = NULL;
                    }
                }
                read_data(file_in, fields, count); // Đọc dữ liệu trước
                //if (count > 0 && strcmp(fields[0][0], "id") != 0) {
                   // int error = 2, row = 0;
                   // ERROR(error, row);
                //}
                convert_data_raw(file_out, fields, count);
                for (int i = 0; i < count; i++) {
                    for (int j = 0; j < FIELD_COUNT; j++) {
                        free(fields[i][j]);
                    }
                    free(fields[i]);
                }
                free(fields);
            }
            break;
        default:
           
            break;
    }

    fclose(file_in);
    fclose(file_out);
    if (log) fclose(log);
    return 0;
}

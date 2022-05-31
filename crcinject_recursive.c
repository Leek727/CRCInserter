#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

int recursive_pad(char *padding, char *pad_ptr, char *updated_crc, char *data, char *crc, int size, int iter, int padding_size);

unsigned int crc32b(unsigned char *message)
{
    int i, j;
    unsigned int byte, crc, mask;

    i = 0;
    crc = 0xFFFFFFFF;
    while (message[i] != 0)
    {
        byte = message[i]; // Get next byte.
        crc = crc ^ byte;
        for (j = 7; j >= 0; j--)
        { // Do eight times.
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
        i = i + 1;
    }
    return ~crc;
}

int main(int argc, char *argv[])
{
    FILE *ptr;
    int size;

    // LOAD FILE DATA ----------------------------------------------------
    if (argc == 2){
        ptr = fopen(argv[1], "r");

    }else{
        printf("One argument expected.\n");
        return 1;
    }
    // check if file exists
    if (NULL == ptr)
    {
        printf("file can't be opened \n");
        return 1;
    }

    // Get file size and allocate memory
    fseek(ptr, 0, SEEK_END); // seek to end of file
    size = ftell(ptr);       // get current file pointer
    fseek(ptr, 0, SEEK_SET);

    unsigned char file_data[size];

    // read from file
    for (int i = 0; i < size; i++)
    {
        file_data[i] = fgetc(ptr);
    }
    fclose(ptr);

    // remove null terminator holy shit kill me
    unsigned char data[size];
    memcpy(data, file_data, size);

    // Get pointers to positions of the padding and crc space ----------------------------------------------------
    int padding_flag = 0;
    char current;
    int counter = 0;
    unsigned char *pad_ptr;
    unsigned char *crc_ptr;
    for (int i = 0; i < size; i++)
    {
        current = data[i];
        if (current == '#')
        {
            if (counter == 0)
            {
                pad_ptr = &data[i];
                padding_flag = 1;
            }
            counter++;
        }
        else if (current == '\n' && padding_flag)
        {
            crc_ptr = &data[i + 1];
            break;
        }
    }
    if (padding_flag == 0)
    {
        printf("File format incorrect!\n");
        return 1;
    }

    // calculate crc of orig without padding - add crc to file and increase padding until it matches
    // CRC CALULATIONS ----------------------------------------------------
    char crc[9];         // orig
    char updated_crc[9]; // crc and padding added
    char padding[5];
    int test = 0;

    // calculate crc of orig file
    sprintf(crc, "%x", crc32b(data));
    memcpy(crc_ptr, crc, 8);
    
    for (int i = 0; i < size; i++){
        printf("%c", data[i]);
    }

    recursive_pad(padding, pad_ptr, updated_crc, data, crc, size, 0, 5);
    return 0;
}

int recursive_pad(char *padding, char *pad_ptr, char *updated_crc, char *data, char *crc, int size, int iter, int padding_size)
{
    
    for (int i = 0; i < 5; i++){
        printf("%c", padding[i]);
    }
    printf("\r");
    if (iter >= padding_size)
    {
        return 1;
    }
    for (int a = 32; a < 128; a++)
    {
        padding[iter] = a;

        // add padding
        memcpy(pad_ptr, padding, 5);

        // calcuate new crc
        sprintf(updated_crc, "%x", crc32b(data));

        // check if adding the crc will change the overall crc
        if (memcmp(crc, updated_crc, 9) == 0)
        {
            printf("\n");
            printf("Crc found!\n");
            printf("CRC : %s\n", crc);
            printf("Updated CRC : %s\n", updated_crc);

            // WRITE TO FILE ----------------------------------------------------
            FILE *file = fopen("test.txt", "w");
            for (int i = 0; i < size; i++)
            {
                printf("%c", data[i]);
                fputc(data[i], file);
            }
            printf("\n");

            fclose(file);
            exit(0);
        }
        recursive_pad(padding, pad_ptr, updated_crc, data, crc, size, iter+1, padding_size);
    }
}
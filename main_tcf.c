#include "data_header/data.h"
#define BUFFER_SIZE 2048 // Define buffer size for reading

// Global variable to control threads
int stop_thread = 0;

// VARIAVEL PARA LIGAR E DESLIGAR O AQUECEDEOR
int ligar_desligar = 0;
// VARIAVEL PARA O SET POINT
double set_point = 0.0, ret_set_point = 0.0;
// VARIAVEL PARA MUDAR A FREQUENCIA
double freq = 0.0, ret_freq = 0.0;

int write_pipe_success(char *pipe)
{
    mkfifo(pipe, 0666);
    int fd = open(pipe, O_WRONLY);
    if (fd == -1)
    {
        perror("Error opening pipe for writing");
        exit(EXIT_FAILURE);
    }
    return fd;
}

int open_pipe_success(char *pipe)
{
    mkfifo(pipe, 0666);
    int fd = open(pipe, O_RDONLY);
    if (fd == -1)
    {
        perror("Error opening pipe for reading");
        exit(EXIT_FAILURE);
    }
    return fd;
}

int index_token_temperature_heater(char *buffer, sensor *s)
{
    int index = 0;
    char *token = buffer;
    while (token != NULL && index < 4)
    {
        char *heater_str = strstr(token, "-");
        if (heater_str != NULL)
        {
            char *temperature_str = token;                     // This points to the temperature part
            char *second_hyphen = strchr(heater_str + 1, '-'); // Find the second hyphen
            if (second_hyphen != NULL)
            {
                *second_hyphen = '\0';                // Split the string at the second hyphen
                char *heater_str = second_hyphen + 1; // Pointer to the heater part
                s->temperatures[index] = atof(temperature_str);
                s->heaters[index] = atoi(heater_str);
            }
            else
            {

                // Convert the temperature from string to double
                s->temperatures[index] = atof(token);
                // Convert the heater status from string to int
                s->heaters[index] = atoi(heater_str);
            }

            if (s->heaters[index] == 0)
            {
                strcpy(s->state_heater[index], "Off");
            }
            else
            {
                strcpy(s->state_heater[index], "On");
            }
            index++;
        }

        token = strtok(NULL, ";");
    }
    return index;
}

void token_temp_heater(char *buffer, int index, sensor *s)
{
    char *token = buffer;
    while (token != NULL && index < 4)
    {
        char *heater_str = strstr(token, "-");
        if (heater_str != NULL)
        {
            char *temperature_str = token;                     // This points to the temperature part
            char *second_hyphen = strchr(heater_str + 1, '-'); // Find the second hyphen
            if (second_hyphen != NULL)
            {
                *second_hyphen = '\0';                // Split the string at the second hyphen
                char *heater_str = second_hyphen + 1; // Pointer to the heater part

                s->temperatures[index] = atof(temperature_str);
                s->heaters[index] = atoi(heater_str);

                // Print the results
            }
            else
            {

                // Convert the temperature from string to double
                s->temperatures[index] = atof(token);
                // Convert the heater status from string to int
                s->heaters[index] = atoi(heater_str);
            }

            if (s->heaters[index] == 0)
            {
                strcpy(s->state_heater[index], "Off");
            }
            else
            {
                strcpy(s->state_heater[index], "On");
            }
            printf("Temperature %d: %.6f Heater: %s\n", index, s->temperatures[index], s->state_heater[index]);
            index++;
        }

        token = strtok(NULL, ";");
    }
    printf("\n");
}
void read_pipe_tsl_on_tcf(char *buffer, sensor *s, int fd)
{
    ssize_t bytesRead = read(fd, buffer, BUFFER_SIZE - 1);
    buffer[bytesRead] = '\0';
    char *token = strtok(buffer, ";");
    sscanf(token, "%d", &s->raw_data);
    printf("Raw data: %d\n", s->raw_data);

    printf("\n");
}

void identify_heater_without_set_point(sensor *s, int index, int desligar_ligar, char *pipe_write_out)
{
    char buffer_heaters[BUFFER_SIZE];
    char temp_str[BUFFER_SIZE]; // Temporary string to hold each heater value
    buffer_heaters[0] = '\0';   // Initialize it as an empty string
    if (desligar_ligar == 1)
    {
        for (int i = 0; i < index; i++)
        {
            // printf("%f %d\n", s->temperatures[i], s->heaters[i]);

            // s->heaters[i] = 1;
            if (s->temperatures[i] < 0.0)
            {
                printf("Temperatura %d: %f deve ligar o aquecedor\n", i, s->temperatures[i]);
                s->heaters[i] = 1;
            }
            else
            {
                printf("Temperatura %d: %f deve desligar o aquecedor\n", i, s->temperatures[i]);
                s->heaters[i] = 0;
            }
            // Format the heater value as a string
            sprintf(temp_str, "%d", s->heaters[i]);

            // If it's not the first value, add a semicolon
            if (i > 0)
            {
                strcat(buffer_heaters, ";");
            }
            strcat(buffer_heaters, temp_str);
        }

        /*int fd = write_pipe_success(pipe_write_out);
        if (fd != -1)
        {
            // Write the string to the pipe
            if (write(fd, buffer_heaters, strlen(buffer_heaters)) == -1)
            {
                perror("Failed to write to pipe");
                exit(EXIT_FAILURE);
            }
            // Close the pipe
            close(fd);
        }*/
    }
    else
    {
        for (int i = 0; i < index; i++)
        {
            if (s->temperatures[i] < 0.0)
            {
                printf("Temperatura %d: %f deve ligar o aquecedor\n", i, s->temperatures[i]);
                s->heaters[i] = 1;
            }
            else
            {
                printf("Temperatura %d: %f deve desligar o aquecedor\n", i, s->temperatures[i]);
                s->heaters[i] = 0;
            }
            // Format the heater value as a string
            sprintf(temp_str, "%d", s->heaters[i]);

            // If it's not the first value, add a semicolon
            if (i > 0)
            {
                strcat(buffer_heaters, ";");
            }
            strcat(buffer_heaters, temp_str);
        }
    }
    printf("Heaters string: %s\n", buffer_heaters);

    int fd2 = open(pipe_write_out, O_WRONLY);
    if (fd2 == -1)
    {
        perror("Failed to open pipe for writing");
        exit(EXIT_FAILURE);
    }

    // Write the string to the pipe
    if (write(fd2, buffer_heaters, strlen(buffer_heaters)) == -1)
    {
        perror("Failed to write to pipe");
        exit(EXIT_FAILURE);
    }

    // Close the pipe
    close(fd2);
}
/*
void execute_pipe_read(char *buffer, sensor *s)
{
    int ligar_desligar = 1;
    mkfifo(TEMP_INFO_PIPE, 0666);
    int fd = open(TEMP_INFO_PIPE, O_RDONLY);
    double set_point = 4.0;
    int frequencia = 1;
    if (fd == -1)
    {
        perror("Error opening pipe for reading");
        exit(EXIT_FAILURE);
    }
    char buffer_heaters[BUFFER_SIZE];
    char temp_str[BUFFER_SIZE]; // Temporary string to hold each heater value
    // LEITURA DO DESLIGAR/LIGAR O AQUECEDEOR
    // printf("Pretende ligar / desligar o aquecedor: \n");
    // scanf("%d", &ligar_desligar);

    if (set_point >= -20.0 && set_point <= 20.0 && ligar_desligar == 1)
    {
        while (1)
        {
            ssize_t bytesRead = read(fd, buffer, BUFFER_SIZE - 1);
            if (bytesRead > 0)
            {
                buffer[bytesRead] = '\0'; // Ensure null termination
                                          // printf("Raw data from pipe: %s\n", buffer); // Debugging line
                buffer_heaters[0] = '\0'; // Initialize it as an empty string
                // char *token = strtok(buffer, ";"); // Get the first token (to ignore it)
                char *token = strtok(buffer, ";");

                // printf("Temperature %d: ", linhas + 1);
                // LISTAS DAS TEMPERATURAS

                sscanf(token, "%d", &s->raw_data);
                printf("Raw data: %d\n", s->raw_data);

                int index = 0;
                while (token != NULL && index < 4)
                {
                    // printf("%s\n", token);
                    char *heater_str = strstr(token, "-");

                    if (heater_str != NULL)
                    {
                        char *temperature_str = token;                     // This points to the temperature part
                        char *second_hyphen = strchr(heater_str + 1, '-'); // Find the second hyphen

                        if (second_hyphen != NULL)
                        {
                            *second_hyphen = '\0';                // Split the string at the second hyphen
                            char *heater_str = second_hyphen + 1; // Pointer to the heater part

                            s->temperatures[index] = atof(temperature_str);
                            s->heaters[index] = atoi(heater_str);

                            // Print the results
                        }
                        else
                        {

                            // Convert the temperature from string to double
                            s->temperatures[index] = atof(token);
                            // Convert the heater status from string to int
                            s->heaters[index] = atoi(heater_str);
                        }

                        if (s->heaters[index] == 0)
                        {
                            strcpy(s->state_heater[index], "Off");
                        }
                        else
                        {
                            strcpy(s->state_heater[index], "On");
                        }
                        printf("Temperature %d: %.6f Heater: %s\n", index, s->temperatures[index], s->state_heater[index]);
                        index++;
                    }

                    token = strtok(NULL, ";");
                }
                printf("\n");

                for (int i = 0; i < index; i++)
                {
                    if (ligar_desligar == 1)
                    {
                        // s->heaters[i] = 1;
                        if (s->temperatures[i] < set_point)
                        {
                            s->heaters[i] = 1;
                        }
                        else
                        {
                            s->heaters[i] = 0;
                        }

                        // Format the heater value as a string
                        sprintf(temp_str, "%d", s->heaters[i]);

                        // If it's not the first value, add a semicolon
                        if (i > 0)
                        {
                            strcat(buffer_heaters, ";");
                        }
                        strcat(buffer_heaters, temp_str);
                    }
                    else
                    {
                        break;
                    }

                    // printf("%.6f\n", s->temperatures[i]);
                }

                // Concatenate the formatted value to the output string

                // printf("Heaters string: %s\n", buffer_heaters);

                int fd2 = open(RESPONSE_PIPE, O_WRONLY);
                if (fd2 == -1)
                {
                    perror("Failed to open pipe for writing");
                    exit(EXIT_FAILURE);
                }

                // Write the string to the pipe
                if (write(fd2, buffer_heaters, strlen(buffer_heaters)) == -1)
                {
                    perror("Failed to write to pipe");
                    exit(EXIT_FAILURE);
                }

                // Close the pipe
                close(fd2);

                printf("\n");
            }
            else if (bytesRead == -1)
            {
                perror("Error reading from pipe");
            }
            printf("Set Point %f e frequencia %d\n", set_point, frequencia);
            usleep(10); // Sleep for 10 milliseconds
        }

        close(fd);
    }
    else
    {
        printf("Os valores do set point tem que se variar entre -20 e 20 e ligar o aquecedor\n");
    }
}*/

void identify_heater_set_point(sensor *s, int index, double set_point, char *pipe_write_out)
{
    char buffer_heaters[BUFFER_SIZE];
    char temp_str[BUFFER_SIZE]; // Temporary string to hold each heater value
    buffer_heaters[0] = '\0';   // Initialize it as an empty string

    for (int i = 0; i < index; i++)
    {
        if (s->temperatures[i] < set_point)
        {
            printf("Temperatura %d: %f deve ligar o aquecedor\n", i, s->temperatures[i]);
            s->heaters[i] = 1;
        }
        else
        {
            printf("Temperatura %d: %f deve desligar o aquecedor\n", i, s->temperatures[i]);
            s->heaters[i] = 0;
        }
        // Format the heater value as a string
        sprintf(temp_str, "%d", s->heaters[i]);

        // If it's not the first value, add a semicolon
        if (i > 0)
        {
            strcat(buffer_heaters, ";");
        }
        strcat(buffer_heaters, temp_str);
    }
    printf("Heaters string: %s\n", buffer_heaters);

    int fd2 = open(pipe_write_out, O_WRONLY);
    if (fd2 == -1)
    {
        perror("Failed to open pipe for writing");
        exit(EXIT_FAILURE);
    }

    // Write the string to the pipe
    if (write(fd2, buffer_heaters, strlen(buffer_heaters)) == -1)
    {
        perror("Failed to write to pipe");
        exit(EXIT_FAILURE);
    }

    // Close the pipe
    close(fd2);
}

void *read_pipe_thread(void *arg)
{
    sensor *s = (sensor *)arg;
    // thread_args_t *args = (thread_args_t *)arg;
    // sensor *s = args->s;
    int fd = open_pipe_success(TEMP_INFO_PIPE);
    char buffer[BUFFER_SIZE];
    int index = 0;
    if (fd != -1)
    {
        while (!stop_thread)
        {
            read_pipe_tsl_on_tcf(buffer, s, fd);
            token_temp_heater(buffer, index, s);
        }
    }

    close(fd);
    return NULL;
}

void *read_pipe_tsl_on_tcf_thread(void *arg)
{
    thread_args_t *args = (thread_args_t *)arg;
    sensor *s = args->s;
    int desl_lig = args->desligar_ligar;
    int fd = open_pipe_success(TEMP_INFO_PIPE);

    char buffer[BUFFER_SIZE];
    int index = 0;

    if (fd != -1)
    {
        while (!stop_thread)
        {
            read_pipe_tsl_on_tcf(buffer, s, fd);
            // token_temp_heater(buffer, index, &s);
            index = index_token_temperature_heater(buffer, s);
            // printf("%d\n", index);

            identify_heater_without_set_point(s, index, desl_lig, RESPONSE_PIPE);
        }
    }

    close(fd);
    return NULL;
}

void *read_pipe_tsl_on_tcf_set_point_thread(void *arg)
{
    thread_args_t *args = (thread_args_t *)arg;
    sensor *s = args->s;
    double s_point = args->set_point;

    int fd = open_pipe_success(TEMP_INFO_PIPE);

    char buffer[BUFFER_SIZE];
    int index = 0;

    if (fd != -1)
    {
        while (!stop_thread)
        {
            read_pipe_tsl_on_tcf(buffer, s, fd);
            // token_temp_heater(buffer, index, &s);
            index = index_token_temperature_heater(buffer, s);
            // printf("%d\n", index);
            identify_heater_set_point(s, index, s_point, RESPONSE_PIPE);
        }
    }

    ret_set_point = s_point;
    close(fd);
    return NULL;
}

void *read_pipe_tsl_on_tcf_frequency_thread(void *arg)
{
    thread_args_t *args = (thread_args_t *)arg;
    sensor *s = args->s;
    double freq = args->frequencia;

    int fd = open_pipe_success(TEMP_INFO_PIPE);
    char buffer[BUFFER_SIZE];
    int index = 0;
    double div_freq = 1.0 / freq;
    int result = (int)(div_freq * 1000000);
    if (fd != -1)
    {
        while (!stop_thread)
        {
            read_pipe_tsl_on_tcf(buffer, s, fd);
            token_temp_heater(buffer, index, s);
            usleep(result);
        }
    }
    ret_freq = freq;
    close(fd);
    return NULL;
}

// SET POINT / LIGAR E DESLIGAR AQUECEDOR

void flush_input_buffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ; // Consume characters until newline
}
int main(void)
{
    pthread_t pipe_thread, heater_thread, set_point_thread, set_frequency;
    // Create the FIFO for reading
    // mkfifo(TEMP_INFO_PIPE, 0666);
    sensor s;
    // execute_pipe_read(buffer, &s);
    //  calc_value_heater_on_off(buffer, &s, index);
    int value;
    char ch;
    int ligar_desligar = 0;
    double point_set = 0.0;
    double freq_hz = 0.0;

    // FD DAS PIPES DE ENTRADA E SAIDA

    thread_args_t *args = malloc(sizeof(thread_args_t));
    if (args == NULL)
    {
        perror("Failed to allocate memory for thread arguments");
        exit(EXIT_FAILURE);
    }

    do
    {
        printf(" .----------------.  .----------------.  .-----------------. .----------------. ");
        printf("| .--------------. || .--------------. || .--------------. || .--------------. |");
        printf("| | ____    ____ | || |  _________   | || | ____  _____  | || | _____  _____ | |");
        printf("| ||_   \\  /   _|| || | |_   ___  |  | || ||_   \\|_   _| | || ||_   _||_   _|| |");
        printf("| |  |   \\/   |  | || |   | |_  \\_|  | || |  |   \\ | |   | || |  | |    | |  | |");
        printf("| |  | |\\  /| |  | || |   |  _|  _   | || |  | |\\ \\| |   | || |  | '    ' |  | |");
        printf("| | _| |_\\/_| |_ | || |  _| |___/ |  | || | _| |_\\   |_  | || |   \\ `--' /   | |");
        printf("| ||_____||_____|| || | |_________|  | || ||_____|\\____| | || |    `.__.'    | |");
        printf("| |              | || |              | || |              | || |              | |");
        printf("| '--------------' || '--------------' || '--------------' || '--------------' |");
        printf(" '----------------'  '----------------'  '----------------'  '----------------'");
        printf("\n+-----------------------------------------------------------------------------+\n");
        printf("|                          PROJECT CRITICAL SOFTWARE                          |\n");
        printf("|-----------------------------------------------------------------------------|\n");
        printf("|                           0. IMPRIMIR RESULTADOS                            |\n");
        printf("|                         1. ATIVAR/DESATIVAR O AQUECEDOR                     |\n");
        printf("|                  2. MUDAR O SET POINT (TEMPERAURAS ENTRE 0 E 20)            |\n");
        printf("|                               3. MUDAR A FREQUENCIA                         |\n");
        printf("|                               4. VER VALORES ATIVOS                         |\n");
        printf("|                                     5. SAIR                                 |\n");
        printf("+-----------------------------------------------------------------------------+\n");
        printf("\nSeleccione uma opcao: ");
        scanf("%d", &value);
        // printf("%d\n", value);
        flush_input_buffer();
        switch (value)
        {
        case 0:
            // getchar();
            system("clear");
            printf("Imprmir Resultados: \n");
            // args->s = &s;
            //
            stop_thread = 0;
            pthread_create(&pipe_thread, NULL, read_pipe_thread, &s);
            // pthread_create(&pipe_thread, NULL, read_pipe_thread, (void *)args);
            ch = getchar();
            if (ch == '\n')
            {
                stop_thread = 1;
                pthread_join(pipe_thread, NULL);
                system("clear");
            }
            break;
        case 1:
            system("clear");
            printf("Ativar/Desativar Aquecedor: \n");
            printf("Pretende ligar/desligar o aquecedor: ");
            scanf("%d", &ligar_desligar);
            flush_input_buffer(); // Ensure to clear any remaining input

            args->s = &s;
            args->desligar_ligar = ligar_desligar;
            stop_thread = 0;
            pthread_create(&heater_thread, NULL, read_pipe_tsl_on_tcf_thread, (void *)args);
            ch = getchar();
            if (ch == '\n')
            {
                stop_thread = 1;
                pthread_join(heater_thread, NULL);
                system("clear");
            }
            //
            break;
        case 2:
            system("clear");
            printf("Obter o Set point: \n");

            printf("Set Point: ");
            scanf("%lf", &point_set);
            flush_input_buffer(); // Ensure to clear any remaining input
            if (point_set >= -20.0 && point_set <= 20)
            {
                args->s = &s;
                args->set_point = point_set;
                stop_thread = 0;
                pthread_create(&set_point_thread, NULL, read_pipe_tsl_on_tcf_set_point_thread, (void *)args);
                ch = getchar();
                if (ch == '\n')
                {
                    stop_thread = 1;
                    pthread_join(set_point_thread, NULL);
                    system("clear");
                }
            }
            else
            {
                printf("Os valores do set point tem que se variar entre -20 e 20.\n");
                flush_input_buffer(); // Ensure to clear any remaining input
            }

            break;
        case 3:
            system("clear");
            printf("Obter A Frequencia (Velocidade de Execução): \n");

            printf("Frequencia de Velocidade: ");
            scanf("%lf", &freq_hz);
            flush_input_buffer(); // Ensure to clear any remaining input
            if (freq_hz >= 1.0 && freq_hz <= 5.0)
            {
                args->s = &s;
                args->frequencia = freq_hz;
                stop_thread = 0;
                pthread_create(&set_frequency, NULL, read_pipe_tsl_on_tcf_frequency_thread, (void *)args);
                ch = getchar();
                if (ch == '\n')
                {
                    stop_thread = 1;
                    pthread_join(set_frequency, NULL);
                    system("clear");
                }
            }
            else
            {
                printf("O valor da frequencia tem que ser entre 1 e 5 Hz.\n");
                flush_input_buffer(); // Ensure to clear any remaining input
            }

            break;
        case 4:
            system("clear");
            printf("Set Point %lf e Frequencia %lf Hz \n", ret_set_point, ret_freq);
            flush_input_buffer(); // Ensure to clear any remaining input
            break;
        }

    } while (value != 5);
    return 0;
}

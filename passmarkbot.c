#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <concord/discord.h>
#include <concord/log.h>

// Struct to hold CPU info read from passmark.txt
typedef struct
{
    char cpuname[256];
    char cores[256];
    char single[256];
    char multi[256];
    char tdp[256];
    char socket[256];
    char type[256];
} CPUSpecs;

typedef struct
{
    char gpuname[256];
    char threedperf[256];
    char twodperf[256];
    char tdp[256];
    char vram[256];
    char type[256];
} GPUSpecs;

// Break the query into tokens and search for each token in the struct & return 1 if all tokens are found (based off of https://www.geeksforgeeks.org/cpp/string-tokenization-in-c/)
int tokenise_query_and_search(const char *fullcpuname, const char *query)
{

    char initialquery[256];
    strcpy(initialquery, query);

    char delimiter[] = " ";

    char *token;

    token = strtok(initialquery, delimiter);

    while (token != NULL)
    {
        if (strcasestr(fullcpuname, token) == NULL)
        {
            return 0;
        }
        token = strtok(NULL, delimiter);
    }

    return 1;
}

void print_usage(void)
{
    printf("\n\n[ Decentcarp's Passmark Discord Bot ]\n\n"
           "Usage:\n"
           "!passmark <CPUNAME> (e.g. !passmark i7-8700K) - Displays single thread perf, multi thread perf, TDP (if applicable).\n"
           "To compare two CPUs, use the format: !passmark <CPU1> | <CPU2> (e.g. !passmark i7-8700K | Ryzen 5 3600).\n"
           "!gpassmark <GPUNAME> (e.g. !gpassmark 7900 XT) - Displays 3D performance, VRAM, TDP (if applicable).\n"
           "To compare two GPUs, use the format: !gpassmark <GPU1> | <GPU2> (e.g. !gpassmark 7900 XT | RTX 3080).\n"
           "!spec <CPUNAME> (e.g !spec Ryzen 5 3600) - Displays cores, single thread perf, multi thread perf, TDP, socket and if its a desktop/mobile/server CPU.\n"
           "!passmark and !gpassmark can be shortened to !pm and !gpm respectively.\n\n"

           "\nPress Enter to start the bot.\n");
}

// Log when the bot connects to Discord
void on_ready(struct discord *client, const struct discord_ready *event)
{
    log_info("Decentcarp's Passmark Discord Bot connected to Discord as %s#%s!",
             event->user->username, event->user->discriminator);
}

void passmark(struct discord *client, const struct discord_message *event)
{

    // query is first CPU (required), query2 is second CPU for comparison, which is optional
    char query[100] = {0};
    char query2[100] = {0};

    // Whitespace trimmer & separate message content into query and query2 with | as the seperator (|, query2 is optional)
    if (event->content && event->content[0] != '\0')
    {
        strncpy(query, event->content, sizeof(query) - 1);

        char *start = query;
        while (*start == ' ' || *start == '\t')
            start++;

        if (start != query)
            memmove(query, start, strlen(start) + 1);
        size_t len = strlen(query);
        while (len > 0 && (query[len - 1] == ' ' || query[len - 1] == '\t' ||
                           query[len - 1] == '\n' || query[len - 1] == '\r'))
        {
            query[--len] = '\0';
        }
        char *pipe = strchr(query, '|');
        if (pipe != NULL)
        {
            *pipe = '\0';
            size_t len = strlen(query);
            while (len > 0 && (query[len - 1] == ' ' || query[len - 1] == '\t' ||
                               query[len - 1] == '\n' || query[len - 1] == '\r'))
            {
                query[--len] = '\0';
            }
            strncpy(query2, pipe + 1, sizeof(query2) - 1);
            char *start2 = query2;
            while (*start2 == ' ' || *start2 == '\t')
                start2++;
            if (start2 != query2)
                memmove(query2, start2, strlen(start2) + 1);
            size_t len2 = strlen(query2);
            while (len2 > 0 && (query2[len2 - 1] == ' ' || query2[len2 - 1] == '\t' ||
                                query2[len2 - 1] == '\n' || query2[len2 - 1] == '\r'))
            {
                query2[--len2] = '\0';
            }
        }
    }

    // Return error if query/message content is empty
    if (query[0] == '\0')
    {
        struct discord_embed_field fields[] = {
            {
                .name = "I need something to search for.",
                .value = "Usage: !passmark <cpu (approximate or exact)> (| <cpu2> ) - for comparison between two CPUs.",
            },
        };

        struct discord_embed embeds[] = {
            {
                .title = ":P",
                .color = 0x3498DB,
                .timestamp = discord_timestamp(client),
                .fields =
                    &(struct discord_embed_fields){
                        .size = sizeof(fields) / sizeof *fields,
                        .array = fields,
                    },
            },
        };

        struct discord_create_message params = {
            .embeds =
                &(struct discord_embeds){
                    .size = sizeof(embeds) / sizeof *embeds,
                    .array = embeds,
                },
        };

        discord_create_message(client, event->channel_id, &params, NULL);

        return;
    }

    // Opens passmark.txt
    FILE *file;

    file = fopen("passmark.txt", "r");

    // Returns error if passmark.txt cannot be found or opened
    if (file == NULL)
    {
        log_info("I couldn't find the specified database file :C");
        return;
    }

    // Init struct & read values from passmark.txt into CPUSpecs struct array, capped at 7000 entries and 7 string values per array entry (it HAS to be 7 values)
    static CPUSpecs cpuspecs[7000];

    int readvalues = 0;
    int numberofcpus = 0;

    while ((readvalues = fscanf(file,
                                "%255[^,],%255[^,],%255[^,],%255[^,],%255[^,],%255[^,],%255[^\r\n]%*[\r\n]",
                                cpuspecs[numberofcpus].cpuname,
                                cpuspecs[numberofcpus].cores,
                                cpuspecs[numberofcpus].multi,
                                cpuspecs[numberofcpus].single,
                                cpuspecs[numberofcpus].tdp,
                                cpuspecs[numberofcpus].socket,
                                cpuspecs[numberofcpus].type)) == 7)
    {
        if (numberofcpus >= 7000)
            break;
        numberofcpus++;
    }

    // Returns error if passmark.txt is not formatted correctly
    if (readvalues != EOF)
    {
        log_info("The database file is not formatted correctly :C\n");
        return;
    }

    // Close passmark.txt as we're done using it
    fclose(file);
    if (event->author->bot)
        return;

    // Read through through CPUSpecs struct array and search for a matching CPU using tokenise_query_and_search, with query as the input
    int i;

    for (i = 0; i < numberofcpus; i++)
    {
        if (tokenise_query_and_search(cpuspecs[i].cpuname, query))
        {

            // If query2 is empty, skip searching query2 and display the results for query
            if (query2[0] == '\0')
            {
                char name[256] = "";
                sprintf(name, "**%s**", cpuspecs[i].cpuname);

                char performance[256] = "";

                // Check if TDP is 0, and if so, don't parse it into the performance string
                if (cpuspecs[i].tdp[0] == '\0' || strcmp(cpuspecs[i].tdp, "0") == 0)
                {
                    sprintf(performance, "**Single:** %s | **Multi:** %s", cpuspecs[i].single, cpuspecs[i].multi);
                }
                else
                {
                    sprintf(performance, "**Single:** %s | **Multi:** %s | **TDP:** %sW", cpuspecs[i].single, cpuspecs[i].multi, cpuspecs[i].tdp);
                }

                // Send an embed containing name (CPU name) and performance (single thread performance, multi thread performance and TDP if not 0)
                struct discord_embed_field fields[] = {
                    {
                        .name = name,
                        .value = performance,
                    },
                };

                struct discord_embed embeds[] = {
                    {
                        .color = 0x3498DB,
                        .timestamp = discord_timestamp(client),
                        .fields =
                            &(struct discord_embed_fields){
                                .size = sizeof(fields) / sizeof *fields,
                                .array = fields,
                            },
                    },
                };

                struct discord_create_message params = {
                    .embeds =
                        &(struct discord_embeds){
                            .size = sizeof(embeds) / sizeof *embeds,
                            .array = embeds,
                        },
                };

                discord_create_message(client, event->channel_id, &params, NULL);
            }
            else
            {

                int i2;

                // Read through through CPUSpecs struct array yet again and search for the second CPU using tokenise_query_and_search, with query2 as the input
                for (i2 = 0; i2 < numberofcpus; i2++)
                {
                    if (tokenise_query_and_search(cpuspecs[i2].cpuname, query2))
                    {
                        // Save first CPU's name and performance into cpu1name and cpu1performance
                        char cpu1name[256] = "";
                        sprintf(cpu1name, "**%s**", cpuspecs[i].cpuname);
                        char cpu1performance[256] = "";

                        // For percentage difference
                        int cpusingle1 = atoi(cpuspecs[i].single);
                        int cpusingle2 = atoi(cpuspecs[i2].single);
                        int cpumulti1 = atoi(cpuspecs[i].multi);
                        int cpumulti2 = atoi(cpuspecs[i2].multi);
                        int sdiff = (cpusingle1 - cpusingle2) * 100 / cpusingle2;
                        int mdiff = (cpumulti1 - cpumulti2) * 100 / cpumulti2;

                        // Check if TDP is 0, and if so, don't parse it into the performance string (for first CPU)
                        if (cpuspecs[i].tdp[0] == '\0' || strcmp(cpuspecs[i].tdp, "0") == 0)
                        {
                            if (cpusingle1 > cpusingle2)
                            {
                                if (cpumulti1 > cpumulti2)
                                {
                                    sprintf(cpu1performance, "**Single:** %s *(+%d%%)* | **Multi:** %s *(+%d%%)*", cpuspecs[i].single, sdiff, cpuspecs[i].multi, mdiff);
                                }
                                else
                                {
                                    sprintf(cpu1performance, "**Single:** %s *(+%d%%)* | **Multi:** %s", cpuspecs[i].single, sdiff, cpuspecs[i].multi);
                                }
                            }
                            else
                            {
                                if (cpumulti1 > cpumulti2)
                                {
                                    sprintf(cpu1performance, "**Single:** %s | **Multi:** %s *(+%d%%)*", cpuspecs[i].single, cpuspecs[i].multi, mdiff);
                                }
                                else
                                {
                                    sprintf(cpu1performance, "**Single:** %s | **Multi:** %s", cpuspecs[i].single, cpuspecs[i].multi);
                                }
                            }
                        }
                        else
                        {
                            if (cpusingle1 > cpusingle2)
                            {
                                if (cpumulti1 > cpumulti2)
                                {
                                    sprintf(cpu1performance, "**Single:** %s *(+%d%%)* | **Multi:** %s *(+%d%%)* | **TDP:** %sW", cpuspecs[i].single, sdiff, cpuspecs[i].multi, mdiff, cpuspecs[i].tdp);
                                }
                                else
                                {
                                    sprintf(cpu1performance, "**Single:** %s *(+%d%%)* | **Multi:** %s | **TDP:** %sW", cpuspecs[i].single, sdiff, cpuspecs[i].multi, cpuspecs[i].tdp);
                                }
                            }
                            else
                            {
                                if (cpumulti1 > cpumulti2)
                                {
                                    sprintf(cpu1performance, "**Single:** %s | **Multi:** %s *(+%d%%)* | **TDP:** %sW", cpuspecs[i].single, cpuspecs[i].multi, mdiff, cpuspecs[i].tdp);
                                }
                                else
                                {
                                    sprintf(cpu1performance, "**Single:** %s | **Multi:** %s | **TDP:** %sW", cpuspecs[i].single, cpuspecs[i].multi, cpuspecs[i].tdp);
                                }
                            }
                        }

                        // Same drill, save second CPU's name and performance into cpu2name and cpu2performance
                        char cpu2name[256] = "";
                        sprintf(cpu2name, "**%s**", cpuspecs[i2].cpuname);
                        char cpu2performance[256] = "";

                        // for percentage difference
                        int cpusingle1b = atoi(cpuspecs[i2].single);
                        int cpusingle2b = atoi(cpuspecs[i].single);
                        int cpumulti1b = atoi(cpuspecs[i2].multi);
                        int cpumulti2b = atoi(cpuspecs[i].multi);
                        int sdiffb = (cpusingle1b - cpusingle2b) * 100 / cpusingle2b;
                        int mdiffb = (cpumulti1b - cpumulti2b) * 100 / cpumulti2b;

                        // Check if TDP is 0, and if so, don't parse it into the performance string (for second CPU)
                        if (cpuspecs[i2].tdp[0] == '\0' || strcmp(cpuspecs[i2].tdp, "0") == 0)
                        {
                            if (cpusingle1b > cpusingle2b)
                            {
                                if (cpumulti1b > cpumulti2b)
                                {
                                    sprintf(cpu2performance, "**Single:** %s *(+%d%%)* | **Multi:** %s *(+%d%%)*", cpuspecs[i2].single, sdiffb, cpuspecs[i2].multi, mdiffb);
                                }
                                else
                                {
                                    sprintf(cpu2performance, "**Single:** %s *(+%d%%)* | **Multi:** %s", cpuspecs[i2].single, sdiffb, cpuspecs[i2].multi);
                                }
                            }
                            else
                            {
                                if (cpumulti1b > cpumulti2b)
                                {
                                    sprintf(cpu2performance, "**Single:** %s | **Multi:** %s *(+%d%%)*", cpuspecs[i2].single, cpuspecs[i2].multi, mdiffb);
                                }
                                else
                                {
                                    sprintf(cpu2performance, "**Single:** %s | **Multi:** %s", cpuspecs[i2].single, cpuspecs[i2].multi);
                                }
                            }
                        }
                        else
                        {
                            if (cpusingle1b > cpusingle2b)
                            {
                                if (cpumulti1b > cpumulti2b)
                                {
                                    sprintf(cpu2performance, "**Single:** %s *(+%d%%)* | **Multi:** %s *(+%d%%)* | **TDP:** %sW", cpuspecs[i2].single, sdiffb, cpuspecs[i2].multi, mdiffb, cpuspecs[i2].tdp);
                                }
                                else
                                {
                                    sprintf(cpu2performance, "**Single:** %s *(+%d%%)* | **Multi:** %s | **TDP:** %sW", cpuspecs[i2].single, sdiffb, cpuspecs[i2].multi, cpuspecs[i2].tdp);
                                }
                            }
                            else
                            {
                                if (cpumulti1b > cpumulti2b)
                                {
                                    sprintf(cpu2performance, "**Single:** %s | **Multi:** %s *(+%d%%)* | **TDP:** %sW", cpuspecs[i2].single, cpuspecs[i2].multi, mdiffb, cpuspecs[i2].tdp);
                                }
                                else
                                {
                                    sprintf(cpu2performance, "**Single:** %s | **Multi:** %s | **TDP:** %sW", cpuspecs[i2].single, cpuspecs[i2].multi, cpuspecs[i2].tdp);
                                }
                            }
                        }

                        // Send an embed containing the two CPU's name and performance, with the first CPUs name and performance being on the first two lines, and the second CPU's name and performance being on the last two lines
                        struct discord_embed_field fields[] = {
                            {
                                .name = cpu1name,
                                .value = cpu1performance,
                            },
                            {
                                .name = cpu2name,
                                .value = cpu2performance,
                            },
                        };

                        struct discord_embed embeds[] = {
                            {
                                .color = 0x3498DB,
                                .timestamp = discord_timestamp(client),
                                .fields =
                                    &(struct discord_embed_fields){
                                        .size = sizeof(fields) / sizeof *fields,
                                        .array = fields,
                                    },
                            },
                        };

                        struct discord_create_message params = {
                            .embeds =
                                &(struct discord_embeds){
                                    .size = sizeof(embeds) / sizeof *embeds,
                                    .array = embeds,
                                },
                        };

                        discord_create_message(client, event->channel_id, &params, NULL);

                        break;
                    }
                }
            };
            break;
        }
        else if (i == numberofcpus - 1)
        {
            // Send message if no matching CPU is found after reading through the entire CPUSpecs struct array
            struct discord_embed_field fields[] = {
                {
                    .name = "I couldn't find anything matching that query.",
                    .value = "Check for typos and try again.",
                },
            };

            struct discord_embed embeds[] = {
                {
                    .title = ":C",
                    .color = 0x3498DB,
                    .timestamp = discord_timestamp(client),
                    .fields =
                        &(struct discord_embed_fields){
                            .size = sizeof(fields) / sizeof *fields,
                            .array = fields,
                        },
                },
            };

            struct discord_create_message params = {
                .embeds =
                    &(struct discord_embeds){
                        .size = sizeof(embeds) / sizeof *embeds,
                        .array = embeds,
                    },
            };

            discord_create_message(client, event->channel_id, &params, NULL);

            break;
        }
    }
}

void gpupassmark(struct discord *client, const struct discord_message *event)
{

    // query is first CPU (required), query2 is second CPU for comparison, which is optional
    char query[100] = {0};
    char query2[100] = {0};

    // Whitespace trimmer & separate message content into query and query2 with | as the seperator (|, query2 is optional)
    if (event->content && event->content[0] != '\0')
    {
        strncpy(query, event->content, sizeof(query) - 1);

        char *start = query;
        while (*start == ' ' || *start == '\t')
            start++;

        if (start != query)
            memmove(query, start, strlen(start) + 1);
        size_t len = strlen(query);
        while (len > 0 && (query[len - 1] == ' ' || query[len - 1] == '\t' ||
                           query[len - 1] == '\n' || query[len - 1] == '\r'))
        {
            query[--len] = '\0';
        }
        char *pipe = strchr(query, '|');
        if (pipe != NULL)
        {
            *pipe = '\0';
            size_t len = strlen(query);
            while (len > 0 && (query[len - 1] == ' ' || query[len - 1] == '\t' ||
                               query[len - 1] == '\n' || query[len - 1] == '\r'))
            {
                query[--len] = '\0';
            }
            strncpy(query2, pipe + 1, sizeof(query2) - 1);
            char *start2 = query2;
            while (*start2 == ' ' || *start2 == '\t')
                start2++;
            if (start2 != query2)
                memmove(query2, start2, strlen(start2) + 1);
            size_t len2 = strlen(query2);
            while (len2 > 0 && (query2[len2 - 1] == ' ' || query2[len2 - 1] == '\t' ||
                                query2[len2 - 1] == '\n' || query2[len2 - 1] == '\r'))
            {
                query2[--len2] = '\0';
            }
        }
    }

    // Return error if query/message content is empty
    if (query[0] == '\0')
    {
        struct discord_embed_field fields[] = {
            {
                .name = "I need something to search for.",
                .value = "Usage: !gpassmark <gpu (approximate or exact)> (| <gpu2> ) - for comparison between two GPUs.",
            },
        };

        struct discord_embed embeds[] = {
            {
                .title = ":P",
                .color = 0x3498DB,
                .timestamp = discord_timestamp(client),
                .fields =
                    &(struct discord_embed_fields){
                        .size = sizeof(fields) / sizeof *fields,
                        .array = fields,
                    },
            },
        };

        struct discord_create_message params = {
            .embeds =
                &(struct discord_embeds){
                    .size = sizeof(embeds) / sizeof *embeds,
                    .array = embeds,
                },
        };

        discord_create_message(client, event->channel_id, &params, NULL);

        return;
    }

    // Opens passmark.txt
    FILE *file;

    file = fopen("gpupassmark.txt", "r");

    // Returns error if passmark.txt cannot be found or opened
    if (file == NULL)
    {
        log_info("I couldn't find the specified database file :C");
        return;
    }

    // Init struct & read values from passmark.txt into GPUSpecs struct array, capped at 2947 entries and 6 string values per array entry (it HAS to be 6 values)
    static GPUSpecs gpuspecs[2947];

    int readvalues = 0;
    int numberofgpus = 0;

    while ((readvalues = fscanf(file,
                                "%255[^,],%255[^,],%255[^,],%255[^,],%255[^,],%255[^\r\n]%*[\r\n]",
                                gpuspecs[numberofgpus].gpuname,
                                gpuspecs[numberofgpus].threedperf,
                                gpuspecs[numberofgpus].twodperf,
                                gpuspecs[numberofgpus].tdp,
                                gpuspecs[numberofgpus].vram,
                                gpuspecs[numberofgpus].type)) == 6)
    {
        if (numberofgpus >= 2947)
            break;
        numberofgpus++;
    }

    // Returns error if passmark.txt is not formatted correctly
    if (readvalues != EOF)
    {
        log_info("The database file is not formatted correctly :C\n");
        return;
    }

    // Close passmark.txt as we're done using it
    fclose(file);
    if (event->author->bot)
        return;

    // Read through through GPUSpecs struct array and search for a matching GPU using tokenise_query_and_search, with query as the input
    int i;

    for (i = 0; i < numberofgpus; i++)
    {
        if (tokenise_query_and_search(gpuspecs[i].gpuname, query))
        {

            // If query2 is empty, skip searching query2 and display the results for query
            if (query2[0] == '\0')
            {
                char name[256] = "";
                sprintf(name, "**%s**", gpuspecs[i].gpuname);
                char performance[256] = "";

                // Check if TDP is 0, and if so, don't parse it into the performance string
                if (gpuspecs[i].tdp[0] == '\0' || strcmp(gpuspecs[i].tdp, "0") == 0)
                {
                    sprintf(performance, "**3D Perf:** %s | **VRAM:** %s", gpuspecs[i].threedperf, gpuspecs[i].vram);
                }
                else
                {
                    sprintf(performance, "**3D Perf:** %s | **VRAM:** %s | **TDP:** %sW", gpuspecs[i].threedperf, gpuspecs[i].vram, gpuspecs[i].tdp);
                }

                // Send an embed containing name (CPU name) and performance (single thread performance, multi thread performance and TDP if not 0)
                struct discord_embed_field fields[] = {
                    {
                        .name = name,
                        .value = performance,
                    },
                };

                struct discord_embed embeds[] = {
                    {
                        .color = 0x3498DB,
                        .timestamp = discord_timestamp(client),
                        .fields =
                            &(struct discord_embed_fields){
                                .size = sizeof(fields) / sizeof *fields,
                                .array = fields,
                            },
                    },
                };

                struct discord_create_message params = {
                    .embeds =
                        &(struct discord_embeds){
                            .size = sizeof(embeds) / sizeof *embeds,
                            .array = embeds,
                        },
                };

                discord_create_message(client, event->channel_id, &params, NULL);
            }
            else
            {

                int i2;

                // Read through through GPUSpecs struct array yet again and search for the second GPU using tokenise_query_and_search, with query2 as the input
                for (i2 = 0; i2 < numberofgpus; i2++)
                {
                    if (tokenise_query_and_search(gpuspecs[i2].gpuname, query2))
                    {

                        // Save first GPU's name and performance into gpu1name and gpu1performance
                        char gpu1name[256] = "";
                        sprintf(gpu1name, "**%s**", gpuspecs[i].gpuname);
                        char gpu1performance[256] = "";
                
                        

                        // for percentage difference
                        int gpuperf1 = atoi(gpuspecs[i].threedperf);
                        int gpuperf2 = atoi(gpuspecs[i2].threedperf);
                        int gdiff = (gpuperf1 - gpuperf2) * 100 / gpuperf2;
                        

                        // Check if TDP is 0, and if so, don't parse it into the performance string (for first GPU)
                        if (gpuspecs[i].tdp[0] == '\0' || strcmp(gpuspecs[i].tdp, "0") == 0)
                        {
                            if (gpuperf1 > gpuperf2)
                            {
                                sprintf(gpu1performance, "**3D Perf:** %s *(+%d%%)* | **VRAM:** %s", gpuspecs[i].threedperf, gdiff, gpuspecs[i].vram);
                            }
                            else
                            {
                                sprintf(gpu1performance, "**3D Perf:** %s | **VRAM:** %s", gpuspecs[i].threedperf, gpuspecs[i].vram);
                            }
                        }
                        else
                        {
                            if (gpuperf1 > gpuperf2)
                            {
                                sprintf(gpu1performance, "**3D Perf:** %s *(+%d%%)* | **VRAM:** %s | **TDP:** %sW", gpuspecs[i].threedperf, gdiff, gpuspecs[i].vram, gpuspecs[i].tdp);
                            }
                            else
                            {
                                sprintf(gpu1performance, "**3D Perf:** %s | **VRAM:** %s | **TDP:** %sW", gpuspecs[i].threedperf, gpuspecs[i].vram, gpuspecs[i].tdp);
                            }
                        }

                        // Same drill, save second GPU's name and performance into gpu2name and gpu2performance
                        char gpu2name[256] = "";
                        sprintf(gpu2name, "**%s**", gpuspecs[i2].gpuname);
                        char gpu2performance[256] = "";
                        
                        // for percentage difference
                        int gpuperf1b = atoi(gpuspecs[i].threedperf);
                        int gpuperf2b = atoi(gpuspecs[i2].threedperf);
                        int gdiffb = (gpuperf2b - gpuperf1b) * 100 / gpuperf1b;
                        
                        if (gpuspecs[i2].tdp[0] == '\0' || strcmp(gpuspecs[i2].tdp, "0") == 0)
                        {
                            if (gpuperf2b > gpuperf1b)
                            {
                                sprintf(gpu2performance, "**3D Perf:** %s *(+%d%%)* | **VRAM:** %s", gpuspecs[i2].threedperf, gdiffb, gpuspecs[i2].vram);
                            }
                            else
                            {
                                sprintf(gpu2performance, "**3D Perf:** %s | **VRAM:** %s", gpuspecs[i2].threedperf, gpuspecs[i2].vram);
                            }
                        }
                        else
                        {
                            if (gpuperf2b > gpuperf1b)
                            {
                                sprintf(gpu2performance, "**3D Perf:** %s *(+%d%%)* | **VRAM:** %s | **TDP:** %sW", gpuspecs[i2].threedperf, gdiffb, gpuspecs[i2].vram, gpuspecs[i2].tdp);
                            }
                            else
                            {
                                sprintf(gpu2performance, "**3D Perf:** %s | **VRAM:** %s | **TDP:** %sW", gpuspecs[i2].threedperf, gpuspecs[i2].vram, gpuspecs[i2].tdp);
                            }
                        }

                        // Send an embed containing the two GPU's name and performance, with the first GPUs name and performance being on the first two lines, and the second GPU's name and performance being on the last two lines
                        struct discord_embed_field fields[] = {
                            {
                                .name = gpu1name,
                                .value = gpu1performance,
                            },
                            {
                                .name = gpu2name,
                                .value = gpu2performance,
                            },
                        };

                        struct discord_embed embeds[] = {
                            {
                                .color = 0x3498DB,
                                .timestamp = discord_timestamp(client),
                                .fields =
                                    &(struct discord_embed_fields){
                                        .size = sizeof(fields) / sizeof *fields,
                                        .array = fields,
                                    },
                            },
                        };

                        struct discord_create_message params = {
                            .embeds =
                                &(struct discord_embeds){
                                    .size = sizeof(embeds) / sizeof *embeds,
                                    .array = embeds,
                                },
                        };

                        discord_create_message(client, event->channel_id, &params, NULL);

                        break;
                    }
                }
            };
            break;
        }
        else if (i == numberofgpus - 1)
        {
            // Send message if no matching GPU is found after reading through the entire GPUSpecs struct array
            struct discord_embed_field fields[] = {
                {
                    .name = "I couldn't find anything matching that query.",
                    .value = "Check for typos and try again.",
                },
            };

            struct discord_embed embeds[] = {
                {
                    .title = ":C",
                    .color = 0x3498DB,
                    .timestamp = discord_timestamp(client),
                    .fields =
                        &(struct discord_embed_fields){
                            .size = sizeof(fields) / sizeof *fields,
                            .array = fields,
                        },
                },
            };

            struct discord_create_message params = {
                .embeds =
                    &(struct discord_embeds){
                        .size = sizeof(embeds) / sizeof *embeds,
                        .array = embeds,
                    },
            };

            discord_create_message(client, event->channel_id, &params, NULL);

            break;
        }
    }
}

void spec(struct discord *client, const struct discord_message *event)
{
    // Query is the CPU to search for
    char query[100] = {0};

    // Same drill, whitespace trimmer but without the seperation part
    if (event->content && event->content[0] != '\0')
    {
        strncpy(query, event->content, sizeof(query) - 1);

        char *start = query;
        while (*start == ' ' || *start == '\t')
            start++;

        if (start != query)
            memmove(query, start, strlen(start) + 1);
        size_t len = strlen(query);
        while (len > 0 && (query[len - 1] == ' ' || query[len - 1] == '\t' ||
                           query[len - 1] == '\n' || query[len - 1] == '\r'))
        {
            query[--len] = '\0';
        }
    }

    // Return error if message content is empty
    if (query[0] == '\0')
    {
        struct discord_embed_field fields[] = {
            {
                .name = "I need something to search for.",
                .value = "Usage: !specs <cpu (approximate or exact)>",
            },
        };

        struct discord_embed embeds[] = {
            {
                .title = ":P",
                .color = 0x3498DB,
                .timestamp = discord_timestamp(client),
                .fields =
                    &(struct discord_embed_fields){
                        .size = sizeof(fields) / sizeof *fields,
                        .array = fields,
                    },
            },
        };

        struct discord_create_message params = {
            .embeds =
                &(struct discord_embeds){
                    .size = sizeof(embeds) / sizeof *embeds,
                    .array = embeds,
                },
        };

        discord_create_message(client, event->channel_id, &params, NULL);

        return;
    }

    // Opens passmark.txt
    FILE *file;

    file = fopen("passmark.txt", "r");

    // Returns error if passmark.txt cannot be found or opened
    if (file == NULL)
    {
        log_info("I couldn't find the specified database file :C");
        return;
    }

    // Init struct & read values from passmark.txt into CPUSpecs struct array, capped at 7000 entries and 7 string values per array entry (it HAS to be 7 values)
    static CPUSpecs cpuspecs[7000];

    int readvalues = 0;
    int numberofcpus = 0;

    while ((readvalues = fscanf(file,
                                "%255[^,],%255[^,],%255[^,],%255[^,],%255[^,],%255[^,],%255[^\r\n]%*[\r\n]",
                                cpuspecs[numberofcpus].cpuname,
                                cpuspecs[numberofcpus].cores,
                                cpuspecs[numberofcpus].multi,
                                cpuspecs[numberofcpus].single,
                                cpuspecs[numberofcpus].tdp,
                                cpuspecs[numberofcpus].socket,
                                cpuspecs[numberofcpus].type)) == 7)
    {
        if (numberofcpus >= 7000)
            break;
        numberofcpus++;
    }

    // Returns error if passmark.txt is not formatted correctly
    if (readvalues != EOF)
    {
        log_info("The database file is not formatted correctly :C\n");
        return;
    }

    // Close passmark.txt as we're done using it
    fclose(file);
    if (event->author->bot)
        return;

    // Read through through CPUSpecs struct array and search for a matching CPU using tokenise_query_and_search, with query as the input
    int i;

    for (i = 0; i < numberofcpus; i++)
    {
        if (tokenise_query_and_search(cpuspecs[i].cpuname, query))
        {
            // If TDP is 0, don't include it in the embed & send embed containing CPU name, cores, type (desktop/mobile/server), socket, single thread performance and multi thread performance.
            if (cpuspecs[i].tdp[0] == '\0' || strcmp(cpuspecs[i].tdp, "0") == 0)
            {
                struct discord_embed_field fields[] = {
                    {
                        .name = "**Name:**",
                        .value = cpuspecs[i].cpuname,
                    },
                    {
                        .name = "**Cores:**",
                        .value = cpuspecs[i].cores,
                    },
                    {
                        .name = "**Type:**",
                        .value = cpuspecs[i].type,
                    },
                    {
                        .name = "**Socket:**",
                        .value = cpuspecs[i].socket,
                    },
                    {
                        .name = "**Single Thread Performance:**",
                        .value = cpuspecs[i].single,
                    },
                    {
                        .name = "**Multi Thread Performance:**",
                        .value = cpuspecs[i].multi,
                    },
                };

                struct discord_embed embeds[] = {
                    {
                        .color = 0x3498DB,
                        .timestamp = discord_timestamp(client),
                        .fields =
                            &(struct discord_embed_fields){
                                .size = sizeof(fields) / sizeof *fields,
                                .array = fields,
                            },
                    },
                };

                struct discord_create_message params = {
                    .embeds =
                        &(struct discord_embeds){
                            .size = sizeof(embeds) / sizeof *embeds,
                            .array = embeds,
                        },
                };

                discord_create_message(client, event->channel_id, &params, NULL);
            }
            else
            {
                // Send embed containing CPU name, cores, type (desktop/mobile/server), socket, TDP, single thread performance and multi thread performance.
                struct discord_embed_field fields[] = {
                    {
                        .name = "**Name:**",
                        .value = cpuspecs[i].cpuname,
                    },
                    {
                        .name = "**Cores:**",
                        .value = cpuspecs[i].cores,
                    },
                    {
                        .name = "**TDP (W):**",
                        .value = cpuspecs[i].tdp,
                    },
                    {
                        .name = "**Type:**",
                        .value = cpuspecs[i].type,
                    },
                    {
                        .name = "**Socket:**",
                        .value = cpuspecs[i].socket,
                    },
                    {
                        .name = "**Single Thread Performance:**",
                        .value = cpuspecs[i].single,
                    },
                    {
                        .name = "**Multi Thread Performance:**",
                        .value = cpuspecs[i].multi,
                    },
                };

                struct discord_embed embeds[] = {
                    {
                        .color = 0x3498DB,
                        .timestamp = discord_timestamp(client),
                        .fields =
                            &(struct discord_embed_fields){
                                .size = sizeof(fields) / sizeof *fields,
                                .array = fields,
                            },
                    },
                };

                struct discord_create_message params = {
                    .embeds =
                        &(struct discord_embeds){
                            .size = sizeof(embeds) / sizeof *embeds,
                            .array = embeds,
                        },
                };

                discord_create_message(client, event->channel_id, &params, NULL);
            };

            break;
        }
    }
}

// Main concord initialisation
int main(int argc, char *argv[])
{
    // Load config.json
    const char *config_file;
    if (argc > 1)
        config_file = argv[1];
    else
        config_file = "./config.json";

    struct discord *client = discord_from_json(config_file);
    assert(NULL != client && "Couldn't initialize client");

    discord_set_on_ready(client, &on_ready);

    // Define passmark and spec commands & their functions with ! as prefix
    discord_set_prefix(client, "!");
    discord_set_on_command(client, "passmark", &passmark);
    discord_set_on_command(client, "pm", &passmark);
    discord_set_on_command(client, "gpassmark", &gpupassmark);
    discord_set_on_command(client, "gpm", &gpupassmark);
    discord_set_on_command(client, "spec", &spec);

    print_usage();
    fgetc(stdin);

    discord_run(client);

    discord_cleanup(client);
}

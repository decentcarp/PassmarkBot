#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
 
#include <concord/discord.h>
#include <concord/log.h>


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

void
print_usage(void)
{
    printf("\n\n[ Decentcarp's Passmark Discord Bot ]\n\n"
           "Usage:\n"
           "!passmark <CPUNAME> (e.g. !passmark i7-8700K) - Displays single thread perf, multi thread perf.\n"
           "To compare two CPUs, use the format: !passmark <CPU1> | <CPU2> (e.g. !passmark i7-8700K | Ryzen 5 3600).\n"
           "!spec <CPUNAME> (e.g !spec Ryzen 5 3600) - Displays cores, single thread perf, multi thread perf, TDP, socket and if its a desktop/mobile/server CPU.\n\n"

           "\nPress Enter to start the bot.\n");
}
 
void
on_ready(struct discord *client, const struct discord_ready *event)
{
    log_info("Decentcarp's Passmark Discord Bot connected to Discord as %s#%s!",
             event->user->username, event->user->discriminator);
}

void
passmark(struct discord *client, const struct discord_message *event)
{

    char query[100] = {0};
    char query2[100] = {0};

    if (event->content && event->content[0] != '\0') {
        strncpy(query, event->content, sizeof(query) - 1);

        char *start = query;
        while (*start == ' ' || *start == '\t')
            start++;

        if (start != query)
            memmove(query, start, strlen(start) + 1);
        size_t len = strlen(query);
        while (len > 0 && (query[len - 1] == ' ' || query[len - 1] == '\t' ||
                           query[len - 1] == '\n' || query[len - 1] == '\r')) {
            query[--len] = '\0';
        }
        char *pipe = strchr(query, '|');
        if (pipe != NULL) {
            *pipe = '\0';
        size_t len = strlen(query);
        while (len > 0 && (query[len - 1] == ' ' || query[len - 1] == '\t' ||
                           query[len - 1] == '\n' || query[len - 1] == '\r')) {
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
                            query2[len2 - 1] == '\n' || query2[len2 - 1] == '\r')) {
        query2[--len2] = '\0';
    }
}
    }

    if (query[0] == '\0') {
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

    if (event->content && event->content[0] != '\0') {
        strncpy(query, event->content, sizeof(query) - 1);

        char *start = query;
        while (*start == ' ' || *start == '\t')
            start++;

        if (start != query)
            memmove(query, start, strlen(start) + 1);
        size_t len = strlen(query);
        while (len > 0 && (query[len - 1] == ' ' || query[len - 1] == '\t' ||
                           query[len - 1] == '\n' || query[len - 1] == '\r')) {
            query[--len] = '\0';
        }
    }
    }
    
    FILE *file;

    file = fopen("passmark.txt", "r");

    if (file == NULL) {
        log_info("I couldn't find the specified database file :C");
        return;
    }

    static CPUSpecs cpuspecs[6224];

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
         cpuspecs[numberofcpus].type)) == 7) {
        if (numberofcpus >= 6224) break;
        numberofcpus++;
    }
  
    if (readvalues != EOF) { 
        log_info("The database file is not formatted correctly :C\n");
        return; 
    }
  
    fclose(file);
    if (event->author->bot) return;
 
    int i;

    for (i = 0; i < numberofcpus; i++) {
        if (strcasestr(cpuspecs[i].cpuname, query) != NULL) {
            if (cpuspecs[i].tdp[0] == '\0' || strcmp(cpuspecs[i].tdp, "0") == 0) {
                if (query2[0] == '\0') { 
                    char name[256] = "";
                    sprintf(name, "**%s**", cpuspecs[i].cpuname);


                    char performance[256] = "";
                    sprintf(performance, "**Single:** %s | **Multi:** %s", cpuspecs[i].single, cpuspecs[i].multi);  

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
                } else { 
                    char cpu1name[256] = "";
                    sprintf(cpu1name, "**%s**", cpuspecs[i].cpuname);
                    
                    char cpu1performance[256] = "";
                    sprintf(cpu1performance, "**Single:** %s | **Multi:** %s", cpuspecs[i].single, cpuspecs[i].multi);  

                    int i2;

                    for (i2 = 0; i2 < numberofcpus; i2++) {
                        if (strcasestr(cpuspecs[i2].cpuname, query2) != NULL) {
                        char cpu2name[256] = "";
                        sprintf(cpu2name, "**%s**", cpuspecs[i2].cpuname);
                        char cpu2performance[256] = "";
                        sprintf(cpu2performance, "**Single:** %s | **Multi:** %s", cpuspecs[i2].single, cpuspecs[i2].multi);  

                        
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
            }            
            } else {
                if (query2[0] == '\0') { 
                    char name[256] = "";
                    sprintf(name, "**%s**", cpuspecs[i].cpuname);


                    char performance[256] = "";
                    sprintf(performance, "**Single:** %s | **Multi:** %s", cpuspecs[i].single, cpuspecs[i].multi);  

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
                } else { 
                    char cpu1name[256] = "";
                    sprintf(cpu1name, "**%s**", cpuspecs[i].cpuname);
                    
                    char cpu1performance[256] = "";
                    sprintf(cpu1performance, "**Single:** %s | **Multi:** %s | **TDP:** %sW", cpuspecs[i].single, cpuspecs[i].multi, cpuspecs[i].tdp);  

                    int i2;

                    for (i2 = 0; i2 < numberofcpus; i2++) {
                        if (strcasestr(cpuspecs[i2].cpuname, query2) != NULL) {
                        char cpu2name[256] = "";
                        sprintf(cpu2name, "**%s**", cpuspecs[i2].cpuname);
                        char cpu2performance[256] = "";
                        sprintf(cpu2performance, "**Single:** %s | **Multi:** %s | **TDP:** %sW", cpuspecs[i2].single, cpuspecs[i2].multi, cpuspecs[i2].tdp);  

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
            } 
        };
        break;
    } else if (i == numberofcpus - 1) {
        struct discord_embed_field fields[] = {
            {
                .name = "I couldn't find anything matching that query.",
                .value = "Check for typos and try again. If you're trying to search with for example 'i3 3210', you need to add the dash. So: 'i3-3210'.",
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

void
spec(struct discord *client, const struct discord_message *event)
{

    char query[100] = {0};
    char query2[100] = {0};

    if (event->content && event->content[0] != '\0') {
        strncpy(query, event->content, sizeof(query) - 1);

        char *start = query;
        while (*start == ' ' || *start == '\t')
            start++;

        if (start != query)
            memmove(query, start, strlen(start) + 1);
        size_t len = strlen(query);
        while (len > 0 && (query[len - 1] == ' ' || query[len - 1] == '\t' ||
                           query[len - 1] == '\n' || query[len - 1] == '\r')) {
            query[--len] = '\0';
        }
    }

    if (query[0] == '\0') {
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
    
    FILE *file;

    file = fopen("passmark.txt", "r");

    if (file == NULL) {
        log_info("I couldn't find the specified database file :C");
        return;
    }

    static CPUSpecs cpuspecs[6224];

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
         cpuspecs[numberofcpus].type)) == 7) {
        if (numberofcpus >= 6224) break;
        numberofcpus++;
    }
  
    if (readvalues != EOF) { 
        log_info("The database file is not formatted correctly :C\n");
        return; 
    }
  
    fclose(file);
    if (event->author->bot) return;
 
    int i;

    for (i = 0; i < numberofcpus; i++) {
        if (strcasestr(cpuspecs[i].cpuname, query) != NULL) {
            if (cpuspecs[i].tdp[0] == '\0' || strcmp(cpuspecs[i].tdp, "0") == 0) {
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

            } else {
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

int
main(int argc, char *argv[])
{
    const char *config_file;
    if (argc > 1)
        config_file = argv[1];
    else
        config_file = "./config.json";

    struct discord *client = discord_from_json(config_file);
    assert(NULL != client && "Couldn't initialize client");

    discord_set_on_ready(client, &on_ready);

    discord_set_prefix(client, "!");
    discord_set_on_command(client, "passmark", &passmark);
    discord_set_on_command(client, "spec", &spec);

    print_usage();
    fgetc(stdin);

    discord_run(client);

    discord_cleanup(client);
}

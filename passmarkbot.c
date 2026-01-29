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
           "Usage: !passmark <CPUNAME>.\n"
           "E.g !passmark i7-8700K\n"
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
                .value = "Usage: !passmark <cpu (approximate or exact)>",
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

                char name[256] = "";
                strcat(name, "**");
                strcat(name, cpuspecs[i].cpuname);
                strcat(name, "**");

                char performance[256] = "";
                strcat(performance, "**Single:** ");
                strcat(performance, cpuspecs[i].single);
                strcat(performance, " | ");
                strcat(performance, "**Multi:** ");
                strcat(performance, cpuspecs[i].multi);

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
                char name[256] = "";
                strcat(name, "**");
                strcat(name, cpuspecs[i].cpuname);
                strcat(name, "**");

                char performance[256] = "";
                strcat(performance, "**Single:** ");
                strcat(performance, cpuspecs[i].single);
                strcat(performance, " | ");
                strcat(performance, "**Multi:** ");
                strcat(performance, cpuspecs[i].multi);
                strcat(performance, " | ");
                strcat(performance, "**TDP:** ");
                strcat(performance, cpuspecs[i].tdp);
                strcat(performance, "W");

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

    print_usage();
    fgetc(stdin);

    discord_run(client);

    discord_cleanup(client);
}

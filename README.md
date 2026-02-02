# Decentcarp's Passmark Discord Bot
An implementation of my [PassmarkSearcher](https://github.com/decentcarp/PassmarkSearcher) as a Discord bot.

![Alt text](https://cdn.discordapp.com/attachments/1460003040424624422/1467922782636474603/image.png?ex=69822520&is=6980d3a0&hm=dad3f0cd4020b07b1e5006ca288d75a1ade3a3a62151d854918f47f829bd96f2&)

## Commands

!passmark - Short, compact way to get the single core performance, multi core performance and the TDP of a CPU.

!spec - Less compact, but also provides the type of CPU (desktop/laptop/server), socket and cores.

## Usage
1. Clone this repo.

2. Build [Concord](https://github.com/Cogmasters/concord/tree/dev)'s dev branch and install Concord.

3. Build passmarkbot.c with ``gcc passmarkbot.c -o passmarkbot -pthread -ldiscord -lcurl``. Alternatively, if your compiler is Clang, you can use ``clang passmarkbot.c -o passmarkbot -pthread -ldiscord -lcurl``.

4. Edit config.json.example by filling in your bot's token and save as config.json.

5. Run and profit.

## Credits

- Decentcarp/Eleanor/R5900 (aka me)
- Concord

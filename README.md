# Decentcarp's Passmark Discord Bot
An implementation of my [PassmarkSearcher](https://github.com/decentcarp/PassmarkSearcher) as a Discord bot.

![Alt text](https://cdn.discordapp.com/attachments/1460003040424624422/1469729781099532359/image.png?ex=6988b806&is=69876686&hm=272a3d189969422e3cc35c90052d3639a591eab3c3106f79729ca98794d468c9&)

## Commands

!passmark - Short, compact way to get the single core performance, multi core performance and the TDP of a CPU. To compare two CPUs with passmark, seperate them with a pipe [|]. (e.g !passmark i7-7700K | i7-9700K)
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

# Decentcarp's Passmark Discord Bot
An implementation of my [PassmarkSearcher](https://github.com/decentcarp/PassmarkSearcher) as a Discord bot.

![Alt text](https://decentcarp.co.uk/images/image.png)

## Commands

!passmark - Get the single core performance, multi core performance and the TDP of a CPU. To compare multiple CPUs with !passmark, seperate them with a pipe [|]. (e.g !passmark i7-7700K | i7-9700K). You may compare up to five CPUs.

!gpassmark - Get the 3D performance, VRAM and the TDP of a GPU. Again, to compare multiple GPUs with !gpassmark, seperate them with a pipe. [|]. (e.g !gpassmark 7900 XT | RTX 3080). You may compare up to five GPUs.

You may also use !pm and !gpm in place of !passmark and !gpassmark respectively.

## Usage
1. Clone this repo.

2. Build [Concord](https://github.com/Cogmasters/concord/tree/dev)'s dev branch and install Concord (``make install`` does it.).

3. Build passmarkbot.c with ``gcc passmarkbot.c -o passmarkbot -pthread -ldiscord -lcurl``. Alternatively, if your compiler is Clang, you can use ``clang passmarkbot.c -o passmarkbot -pthread -ldiscord -lcurl``.

4. Edit config.json.example by filling in your bot's token and save as config.json.

5. Run and profit.

## Credits

- Decentcarp/Eleanor/R5900 (aka me)
- Concord (Discord API bindings)
- [GeeksforGeeks](https://www.geeksforgeeks.org/) (public code reference)
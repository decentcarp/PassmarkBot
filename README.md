# Decentcarp's Passmark Discord Bot
An implementation of my [PassmarkSearcher](https://github.com/decentcarp/PassmarkSearcher) as a Discord bot.

## Usage
1. Clone this repo.

2. Build [Concord](https://github.com/Cogmasters/concord/tree/dev) and install Concord to the repo's directory by setting PREFIX (you can do this by modifying the Makefile, just set PREFIX to the cloned repos directory)

3. Build passmarkbot.c with ``gcc passmarkbot.c -o passmarkbot -pthread -ldiscord -lcurl``. Alternatively, if your compiler is Clang, you can use ``clang passmarkbot.c -o passmarkbot -pthread -ldiscord -lcurl``.

4. Edit config.json.example by filling in your bot's token and save as config.json.

5. Run and profit.

## Credits

- Me
- Concord
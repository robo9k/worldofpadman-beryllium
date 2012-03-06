               _____             _ _ _           
              | __  |___ ___ _ _| | |_|_ _ _____ 
              | __ -| -_|  _| | | | | | | |     |
              |_____|___|_| |_  |_|_|_|___|_|_|_|
                            |___|                

  :: About

  Beryllium is a serverside mod which strives to add important
  administration functionality. This also means to fix existing
  bugs or provide options to circumvent them.
  It is however not meant to add new features to the gameplay.

  If you do not change any of the default settings, Beryllium
  will almost behave as the vanilla game.

  This readme is for Beryllium __VERSION__ and may thus not apply
  to older versions.
  This version is based on World of Padman v1.6 and will therefore
  not work with older, newer or beta versions of the game. This is
  a limitation of the Quake 3 QVM system.


  :: Contact

  If you have any questions or suggestions regarding Beryllium, just
  contact me at brain@loop8.net

  You can also request the source code, as per GPL requirements.


  :: Installation

  According to Beryllium's aims outlined above, you should not
  install Beryllium into a seperate mod folder.

  This means you would copy the included "vm/qagame.qvm" into
  "~/.padman/wop/vm/qagame.qvm" on a Linux system.
  Windows and Mac locations are similar, please refer to the
  World of Padman readme.

  Since Beryllium is a serverside mod, players will not need to
  download anything and your server can remain pure.

  Do not hesitate to take a look at the included "cfg/beryllium.cfg"
  for some recommended settings.


  :: Variables

  Please note that beryllium does not check cvars for sane
  values, since it is assumed that admins know what they
  are doing :)

    :: g_beryllium
    A read-only cvar that indicates the beryllium version.


    :: g_version
    A read-only cvar that indicates the codebase being used.


    :: be_voteDuration
    How long a vote lasts. Time is in seconds. Will only take
    effect for votes called after changing the cvar.


    :: be_allowedVotes
    List of allowed votes. Each vote name needs to be
    enclosed in "/".
    Take a look at the default value.


    :: be_votePause
    General pause between votes. Time is in seconds.


    :: be_voteRate
    How often a client can call a vote. Time is in seconds.


    :: be_votePass
    How many clients need to vote yes for a vote to pass.
    This is a minimum, i.e. the actual number of clients has
    to be above.
    This is a fraction from 0.0 to 1.0


    :: be_respawnProtect
    After respawning, clients don't take and handle damage.
    Time is in seconds. There is no notification whether you
    or a target are (still) protected.


    :: be_maxVotes
    Maximum number of votes per client. This gets reset at
    map change or restart.


    :: be_switchTeamTime
    Minimum pause between team changes for players in seconds.


    :: be_maxNameChanges
    Number of times a client can rename. Does not apply while
    the client is connecting.
    Will stick to the last name before hitting limit.
    Set to -1 for unlimited (default vanilla behaviour) renames.


    :: be_maxConnections
    Maximum number of connections per IP, connections above
    the limit are rejected with an error message.
    This defeats tools like q3fill almost entirely (you could
    furthermore set sv_timeout to a low value).
    If set to default of 0, unlimited connections are allowed.


    :: be_checkGUIDs
    This is a bitmask, supporting the following flags:
      1 - Reject empty GUIDs
      2 - Reject invalid GUIDs

    With broken game installations, the GUID of players may
    sometimes be empty, but this is a rare case.
    Cheat tools like aimbots etc. tend to change the GUID to
    invalid values.
    A valid GUID is exactly 32 chars long and only consists
    of 0-9 and A-F.

    Please also read ioquake's readme concerning the purpose
    of cl_guid!


    :: be_campDistance
    If greater than zero, this is the the distance in units
    the players have to move each second. If they do not, they
    are considered camping and after 20 seconds killed.


    :: be_checkPings
    Will check the ping of each connected and playing player
    every second and kicks if the ping is not within sv_minping
    and sv_maxping or if there were no recent packets for longer
    than the variable's value.


    :: be_oneUp
    Players get an extra life in LPS each be_oneUp kills.


    :: be_noSecrets
    If enabled, players can no longer use specific teleporters
    and movers.
    When using disabled teleporters, players will be taken to a
    random spawnpoint instead.

    Changes will only take effect when loading a new map. Watch
    out for error messages ;)
    Take a look at the included "maps/wop_diner.cfg" as an example.
    The filename pattern is "maps/mapname.cfg".


    :: be_debugSecrets
    Will print debug output to players using teleporters and
    movers, which can be used for be_noSecrets.
    This cvar has the cheat flag and can thus only be set in
    combination with devmap.


    :: be_hideChat
    If set, will hide chat prefixed with this string. E.g. if you
    set it to "!", chat text like "!kick me" will only be written
    to the logfile and echoed back to you, but not be send to
    anyone else.
    This is useful to reduce chat spam when using tools like B3.


    :: be_banFile
    Filename for Beryllium's GUID bans.
    This file will be overwritten on several events.


    :: g_truePing
    Taken from the unlagged code, will do exactly the same.
    Read their readme if you wish to known more.


    :: be_chatFlags
    This is a bitmask, supporting the following flags:
      1 - Swap say and say_team
      2 - Spectators can only use say_team (and tell)


    :: be_overrideEntities
    When activated, will look for a file to replace the map's entities.
    Take a look at the "Overriding entities" section below.


    :: be_settings
    This is a bitmask, supporting the following flags:
      1 - Enforce unique player names
      2 - Enforce simple player names
          Black ^0 is now usable either way


    :: g_smoothClients
    Taken from the unlagged code, will do exactly the same.
    Read their readme if you wish to known more.


    :: be_welcomeMessage
    Will print this message to players on first connect.
    Defaults to an unobtrusive Beryllium version info.




  :: Commands

    :: Client

    :: callvote shuffleteams
    Will do the same as the server command. Enabled by default,
    unless blacklisted with be_allowedVotes.


    :: ignore [cid]
    Allows to ignore any text chat from a given client.
    Without arguments the current list of ignored players will
    be printed.
    Given a client id, the client will either be ignored or
    unignored, it works as a toggle.
    Note that upon disconnect, clients are automatically unignored.
    You can use -1 as client id to ignore all bots.
    If the server supports it, VoIP is also disabled for
    the targets.




    :: Server

    :: stell cid text
    Serverside tell chat.


    :: ssay text
    Serverside say chat.


    :: ssay_team team text
    Serverside team chat. Use r,b,s,f as team name.


    :: scp cid text
    Centerprint text to a client.
    Text will be printed in large letters in the middle of
    the screen.


    :: smp cid text
    Messageprint text to a client.
    Text will be printed in the upper right of the screen.
    Please note that this is also used by the game in BigBalloon to
    indicate Balloons' status.
  
  
    :: sprint cid text
    Print text to a client.
    Text will be printed to the upper left chat area.


    You can use -1 as client id for scp, smp and sprint to send
    to every client.
    Messages sent with stell, ssay and ssay_team will use
    "server" as the player name.

    You can print newlines with scp and sprint by using "\n" in
    the text, which will get expanded to a real newline, e.g.
      scp -1 "line one\nline two"


    :: dropclient cid [reason]
    Basically the same as clientkick, but you can supply an
    additional argument which will be printed.
  
  
    :: cancelvote
    Cancel a currently running vote, i.e. emulate that everyone
    voted no.


    :: shuffleteams
    In team gametypes try to even teams by splitting players
    evenly across teams.
    This does not restart the game or reset scores, except on
    players (since their scores are currently not saved on
    team changes).


    :: rename cid newname
    Rename given client to newname.
    be_maxNameChanges setting does not apply to this, other
    limits like invalid characters in name or multiple names
    do still apply (afterwards)!
    This does not add to the client's rename counter.


    :: scallvote vote
    Almost the same as client's callvote, but some restrictions
    do not apply.


    :: rehashguids
    Reload GUID bans from disk.


    :: listguids
    Print all GUID bans.


    :: flushguids
    Delete all GUID bans, also on disk.


    :: banguid cid|guid
    Add a GUID to the list, either using a client id of a connected player, or
    by supplying the GUID directly.


    :: delguid id|guid
    Delete a GUID from the list, either by using an id from listguids or by
    supplying the GUID directly.


    :: handicap cid handicap
    Sets a players handicap value. Will take effect immediately.
    The player can still decide to override his setting again.


    :: forceteam cid team
    Forces a player into the given team regardsless of any limits.


    :: lockteam team
    Locks or unlocks the given team. Teams will remain locked until nextmap.


    :: sound filename
    Plays the sound globally on the server. Be aware that there is a limit
    of the maximum number of sounds. If you reach it, the server will crash.


    :: beryllium
    Prints version information about beryllium.




  :: Other changes
  
  Beryllium also includes some changes to the game which are
  mandatory and can not be configured at all.
  
   * Several names are now forbidden, e.g. "server".
   * Quite a few sanity checks are being run against votes.
   * Some bugs in the original code have been fixed
   * Most bugfixes from WoP's bleeding edge code have been 
     backported




  :: Overriding entities

  When enabling be_overrideEntities, Beryllium will no load the worldspawn
  from the map's .bsp file, but use a replacement instead.
  Take a look at the included "maps/wop_journeyctl.dat" as an example.
  The filename pattern is "maps/mapname.dat".

  To get dump the worldspawn of a map, use tools/entdmp.
  This file is the same as http://r-1.ch/entdump.c but BSPVERSION was modified
  to match Quake 3.

    gcc tools/entdmp -o entdmp
    ./entdmp maps/wop_journeyCTL.bsp > maps/wop_journeyctl.dat

  You can then edit the created .dat file. Be carefull not to break things!
  Some changes or additions will work, others do not. Maybe entity type
  support will be enhanced, meanwhile figure this out by yourself.



  :: Credits

  Thanks in no particular order:
   * World of Padman Team
   * Community around the game
   * ioquake
   * id Software

  Beryllium uses portions of other people's code, which was
  licensed under the GPL or a similar license.
  Furthermore, some ideas have been adopted or copied.

  Thus additional credits go to:
   * Tremulous
   * OpenArena
   * Smokin' Guns
   * Urban Terror
   * ExcessivePlus
   * ETpro
   * BigBrotherBot
   * r1ch



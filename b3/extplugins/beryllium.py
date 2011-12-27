##
## Beryllium plugin for BigBrotherBot (bigbrotherbot.net)
## Copyright (C) 2011 thbrn
## 
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
##
##
## Changelog:
##
## 19:00 11.06.2011 by thbrn
##  * Added command beryllium, which displays plugin version and author
##  * Added command cancelvote, which allows to cancel the running vote
##  * Added command shuffleteams, which shuffles teams in team gametypes
##  * Added command callvote, which will call a server vote instead of a regular
##    client one
##  * Added centerprint command, which prints centered text to every player
##  * Added rename command, which allows to rename players ingame
##
## 23:30 11.06.2011 by thbrn
##  * Added TODO section
##  * Changed a few misleading or wrong comments
##  * Removed tests, since they did not work without a real game server anyways
##  * Changed some debug output to errors, removed superfluous debug output
##  * Added more error handling for config loading
##
## 00:30 11.06.2011 by thbrn
##  * Added forceteam command, which forcefully moves players into a team
##
## 01:15 11.06.2011 by thbrn
##  * Updated command descriptions, hopefully following "standards" this time
##  * Added info print for commands which affect players
##  * Fixed callvote for votes with arguments
##
## 12:30 12.06.2011 by thbrn
##  * Made Changelog more consistend
##  * Made !rcon and !callvote spit out possible commands if none given
##  * Changed versioning
##  * Added NOTES section
##
## 15:00 12.06.2011 by thbrn
##  * Added a README
##  * Moved beryllium.xml to plugin_beryllium.xml
##
## 16:00 12.06.2011 by thbrn
##  * Split rcon results at linebreak and print in separate chunks.
##    General problem; e.g. !rc devmap foo spits out all the output when loading
##    a map and does not print it to server console anymore, since redirected
##    due to being invoked by rcon. Not quite intended behaviour
##  * !beryllium now also prints mod version
##  * Check and warn at startup, if mod could not be detected
##
## 00:15 15.06.2011 by thbrn
##  * Rewrote printLines() loop logic
##  * Reported bug in q3::abstractParser.getCvar()
##
## 22:50 07.10.2011 by thbrn
##  * Added command playsound
##  * Added command runas
##  * Added command lockteam
##  * Added command handicap
##
## 17:00 08.10.2011 by thbrn
##  * Added command maps
##
##
## TODO:
##
##  * Most rcon results are scrambled when being output, report feature request
##    for getWrap() to consider \n as well. Currently those newlines seem to be
##    dropped (and would not work with client cgame print anyways).
##  * Add remaining beryllium commands (smp, sprint)?
##  * Add more default rcon commands to config
##  * Add handy !rcon "aliases" for nextmap etc.?
##  * Add commands to alter be_oneUp, be_noSecrets?
##  * "Adapt" rotation manager from poweradminurt?
##  * Since this plugin will ony work with World of Padman and the beryllium mod
##    maybe add a .diff for an adjusted parser as well?
##    Currently only change would be 'dropclient %(cid)s %(reason)s' instead of
##    vanilla clientkick
##  * Make error messages to clients more specific about what data is missing
##  * Find a way to add help for !rcon and !callvote sub-functions?
##  * Filter rcon responses? We rely on beryllium mod anyways, so we could
##    could add some regexp as well
##    This obviously won't work for !rcon, since it's meant to be dynamic
##  * Don't require a config, make bare metal commands have a fallback if not
##    present (i.e. no votes, rcon).
##    This is close to make the mod optional
##
##
## NOTES:
##  * You should read beryllium mod's README, really!
##  * beryllium converts "\n" into a real \n, see cmd_rename(), cmd_forceteam()
##  * The entire voting *could* be done with b3 and rcon, see b3 voting plugin,
##    but why reinvent the wheel (and all sanity checks)
##
##


## major.minor.(svn)revision
__version__ = '0.8.2'
__author__  = 'thbrn'

import b3
import b3.plugin
import re


##------------------------------------------------------------------------------


class BerylliumPlugin(b3.plugin.Plugin):
    ## this is the default, just here to be explicitly verbose
    requiresConfigFile = True

    _adminPlugin = None
    _votes = {}
    _rcon = {}
    _maps = []

    ## NOTE: b3 has a verbose warning if no handle(), but at
    ##       same time says "Depreciated. Use onEvent()"


    def startup(self):
        """\
        Initialize plugin settings
        """

        ## get the admin plugin so we can register commands
        self._adminPlugin = self.console.getPlugin('admin')
        if not self._adminPlugin:
            ## something is wrong, can't start without admin plugin
            self.error('Could not find admin plugin')
            return False

        ## TODO: We could check whether g_beryllium cvar is present and
        ##       either bail out or disable beryllium specific commands.
        ##       This would even allow syncing mod<->plugin versions.

        ## Current target mod version: 1.6a-ce5a825

        ## see whether beryllium mod in installed
        try:
            be_version = self.console.getCvar('g_beryllium').getString()
            self.debug('Beryllium mod %s' % be_version)
        except:
            self.warning('Beryllium mod is not running')

        ## register our commands
        if 'commands' in self.config.sections():
            for cmd in self.config.options('commands'):
                try:
                    ## TODO: This prevents "None" to disable the command in config
                    level = self.config.getint('commands', cmd)
                except:
                    self.error('No valid level for command %s, skipping', cmd)
                    continue
                sp = cmd.split('-')
                alias = None    
                if len(sp) == 2:
                    cmd, alias = sp

                func = self.getCmd(cmd)
                if func:
                    self._adminPlugin.registerCommand(self, cmd, level, func, alias)
                else:
                    self.error('Can not register command %s, no associated function', cmd)

        ## version command is not in config, always add
        self._adminPlugin.registerCommand(self, 'beryllium', 40, self.cmd_version)

        self.registerEvent(b3.events.EVT_GAME_ROUND_START)
        self.loadMaps()


    def onLoadConfig(self):
        self.LoadCallvote()
        self.LoadRcon()


    def LoadCallvote(self):
        ## VIP votes setup
        self.debug('Loading callvote config')
        if 'callvote' in self.config.sections():
            for cmd in self.config.options('callvote'):
                try:
                    level = self.config.getint('callvote', cmd)
                except:
                    self.error('No valid level for vote %s, skipping', cmd)
                    continue

                ## TODO: We could hardlcode the list of possible votes and check
                ##       whether config entries match.
                self._votes[cmd] = level
                self.debug('Added vote %s with level %s', cmd, level)


    def LoadRcon(self):
        ## rcon pass-through setup
        self.debug('Loading rcon config')
        if 'rcon' in self.config.sections():
            for cmd in self.config.options('rcon'):
                try:
                    level = self.config.getint('rcon', cmd)
                except:
                    self.error('No valid level for rcon command %s, skipping', cmd)
                    continue

                ##TODO: We could have a regex to at least check whether config
                ##      entries have the proper type for rcon commands, i.e.
                ##      no spaces and exotic chars.
                self._rcon[cmd] = level
                self.debug('Added rcon command %s with level %s', cmd, level)


    def getCmd(self, cmd):
        ## Returns class method cmd prefixed with "cmd_"
        cmd = 'cmd_%s' % cmd
        if hasattr(self, cmd):
            func = getattr(self, cmd)
            return func
        return None


    def cmd_version(self, data, client, cmd=None):
        """\
        prints beryllium mod version, plugin version and creator
        """

        ## This won't work correct as long as getCvar() is bugged
        try:
            be_version = self.console.getCvar('be_version').getString()
        except:
            be_version = '^1NOT-RUNNING'
        client.message('^7This is beryllium ^2%s, ^7plugin version ^4%s^7 by ^5%s^7.' % (be_version, __version__, __author__))

        return None


    def cmd_cancelvote(self, data, client, cmd=None):
        """\
        cancels the running vote
        """

        result = self.console.write('cancelvote')
        self.printLines(client, result.split('\n'))
        ## TODO: Print info to players on success?
        return None


    def cmd_shuffleteams(self, data, client, cmd=None):
        """\
        shuffles teams in team gametypes
        """

        result = self.console.write('shuffleteams')
        self.printLines(client, result.split('\n'))
        ## TODO: Print info to players on success?
        return None


    def cmd_callvote(self, data, client, cmd=None):
        """\
        <vote> [options] - calls the vote as server, options depend on the specific vote.
        Will print a list of possible votes of none given.
        """

        input = self._adminPlugin.parseUserCmd(data)

        ## no vote command given, most likely wants a list of votes
        if not input:
            votes = []
            for cmd,level in self._votes.items():
                if level > client.maxLevel:
                    continue
                votes.append(cmd)
            votes.sort()
            ## TODO: Different message if none
            client.message('^7Available votes: %s' % ', '.join(votes))
            return True

        votecmd = input[0]

        try:
            level = self._votes[votecmd]
        except KeyError:
            client.message('Unknown or disabled vote %s' % votecmd)
            return False

        if level > client.maxLevel:
            client.message('Your level is not high enough to callvote %s' % votecmd)
            return False

        result = self.console.write('scallvote %s' % data)
        self.printLines(client, result.split('\n'))
        ## TODO: on success "runas client vote yes"
        return True


    def cmd_rcon(self, data, client, cmd=None):
        """\
        <command> [options] - will execute command via rcon, options depend on the specific command.
        Will print a list of possible commands if none given.
        """

        input = self._adminPlugin.parseUserCmd(data)

        ## no rcon command given, most likely wants a list of commands
        if not input:
            rcommands = []
            for cmd,level in self._rcon.items():
                if level > client.maxLevel:
                    continue
                rcommands.append(cmd)
            rcommands.sort()
            ## TODO: Different message if none
            client.message('^7Available rcon commands: %s' % ', '.join(rcommands))
            return True

        rconcmd = input[0]

        try:
            level = self._rcon[rconcmd]
        except KeyError:
            client.message('Unknown or disabled rcon command %s' % rconcmd)
            return False

        if level > client.maxLevel:
            client.message('Your level is not high enough to execute rcon command %s' % rconcmd)
            return False

        result = self.console.write(data)
        self.printLines(client, result.split('\n'))

        return True


    def cmd_centerprint(self, data, client, cmd=None):
        """\
        <text> - print centered text to every player
        """

        ## TODO: Make this work with specific clients and -1

        if not data:
            client.message('^7Missing text, try !help centerprint')
            return False

        self.console.write('scp -1 "%s"' % data)
        return True


    def cmd_rename(self, data, client, cmd=None):
        """\
        <player> <newname> - forcefully renames a player, without decreasing his number of remaining renames
        """

        input = self._adminPlugin.parseUserCmd(data)
        if not input:
            client.message('^7Missing data, try !help rename')
            return False

        if not input[0]:
            client.message('^7Missing player, try !help rename')
            return False

        sclient = self._adminPlugin.findClientPrompt(input[0], client)
        if not sclient:
            ## findClientPrompt() already displayed a message to client, just return
            return False

        if not input[1]:
            client.message('^7Missing newname, try !help rename')
            return False


        self.console.write('rename %s "%s"' % (sclient.cid, ' '.join(input[1:])))
        ## TODO: Don't use a hardcoded message and command
        self.console.write('scp %s "^3You were renamed\\n^3by admin"' % sclient.cid)
        return None


    def cmd_forceteam(self, data, client, cmd=None):
        """\
        <player> <red/blue/free/spectator> - forcefully move a player into given team
        """

        input = self._adminPlugin.parseUserCmd(data)
        if not input:
            client.message('^7Missing data, try !help forceteam')
            return False

        if not input[0]:
            client.message('^7Missing player, try !help forceteam')
            return False

        sclient = self._adminPlugin.findClientPrompt(input[0], client)
        if not sclient:
            ## findClientPrompt() already displayed a message to client, just return
            return False

        if not input[1]:
            client.message('^7Missing team, try !help forceteam')
            return False

        if not input[1] in ['red', 'r', 'blue', 'b', 'spectator', 's', 'free', 'f' ]:
            client.message('^7Invalid team, try !help forceteam')
            return False

        self.console.write('forceteam %s %s' % (sclient.cid, input[1]))
        ## TODO: Don't use a hardcoded message and command
        ## TODO: Print result to user to see failure messages
        self.console.write('scp %s "^3You were moved to\\n^3another team by an admin"' % sclient.cid)
        return True


    def cmd_playsound(self, data, client, cmd=None):
        """\
        <filename> - plays a global sound on the server
        """

        input = self._adminPlugin.parseUserCmd(data)
        if not input:
            client.message('^7Missing data, try !help playsound')
            return False

        if not input[0]:
            client.message('^7Missing filename, try !help playsound')
            return False

        self.console.write('sound %s' % input[0])
        ## TODO: Print result to user to see failure messages
        return True


    def cmd_runas(self, data, client, cmd=None):
        """\
        <player> <command> - runs a command as another player
        """

        input = self._adminPlugin.parseUserCmd(data)
        if not input:
            client.message('^7Missing data, try !help runas')
            return False

        if not input[0]:
            client.message('^7Missing player, try !help runas')
            return False

        sclient = self._adminPlugin.findClientPrompt(input[0], client)
        if not sclient:
            ## findClientPrompt() already displayed a message to client, just return
            return False

        if not input[1]:
            client.message('^7Missing command, try !help runas')
            return False

        ## TODO: Concat all remaining input
        self.console.write('runas %s %s' % (sclient.cid, input[1]))
        return True


    def cmd_lockteam(self, data, client, cmd=None):
        """\
        <team> - locks or unlocks the team, preventing players from joining it
        """

        input = self._adminPlugin.parseUserCmd(data)
        if not input:
            client.message('^7Missing data, try !help lockteam')
            return False

        if not input[0]:
            client.message('^7Missing team, try !help lockteam')
            return False

        if not input[0] in ['red', 'r', 'blue', 'b', 'spectator', 's', 'free', 'f' ]:
            client.message('^7Invalid team, try !help lockteam')
            return False

        self.console.write('lockteam %s' % input[0])
        ## TODO: Don't use a hardcoded message and command
        self.console.write('scp -1 "^3Admin (un-)locked\\n^3team %s"' % input[0])
        ## TODO: Print result to user to see failure messages
        return True


    def cmd_handicap(self, data, client, cmd=None):
        """\
        <player> <handicap> - sets the player's handicap
        """

        input = self._adminPlugin.parseUserCmd(data)
        if not input:
            client.message('^7Missing data, try !help handicap')
            return False

        if not input[0]:
            client.message('^7Missing player, try !help handicap')
            return False

        sclient = self._adminPlugin.findClientPrompt(input[0], client)
        if not sclient:
            ## findClientPrompt() already displayed a message to client, just return
            return False

        if not input[1]:
            client.message('^7Missing handicap, try !help handicap')
            return False

        #if not input[1] in range(101):
        #    client.message('^7Handicap not in range 0-100')
        #    return False

        self.console.write('handicap %s %s' % (sclient.cid, input[1]))
        ## TODO: Print result to user to see failure messages
        self.console.write('scp %s "^3An admin set\\n^3handicap for you"' % sclient.cid)
        return True


    def cmd_maplist(self, data, client, cmd=None):
        """\
        [searchterm] - displays maps available on the server
        """

        results = []
        input = self._adminPlugin.parseUserCmd(data)
        if input and input[0]:
            for map in self._maps:
                if map.find(input[0]) <> -1:
                    results.append(map)
        else:
            results = self._maps
        

        self.printLines(client, results)
        return True


    def printLines(self, client, lines):
        ## Prints a bunch of lines to a client

        ## ioquake3 server won't send commands longer than 1022 chars
        ## console.write() will strip whitespace if not quoted
        ## So what we need to do is construct quoted chunks which do
        ## not exceed the command limit and send each one
        ## Plus we'll need to use beryllium mod's sprint with modified
        ## newlines

        maxlen = 1022

        if not client:
            cid = -1
        else:
            cid = client.cid

        if not lines:
            return None

        buff = ''
        for l in lines:
            if len(buff) > 0:
                toadd = '\\n' + l
            else:
                toadd = l

            if len('sprint %s "%s%s"' % (cid,buff,toadd)) < maxlen:
                buff = buff + toadd
            else:
                self.console.write('sprint %s "%s"' % (cid,buff))
                buff = ''

        if len(buff) > 0:
            self.console.write('sprint %s "%s"' % (cid,buff))


    def onEvent(self, event):
        if event.type == b3.events.EVT_GAME_ROUND_START:
            loadMaps()


    def loadMaps(self):
        mapnameRegexp = re.compile(r'^(?P<mapname>.+)\.bsp$', re.IGNORECASE)
        maplist = self.console.write('dir maps bsp')
        self._maps = []

        for line in maplist.split('\n'):
            match = re.match(mapnameRegexp, line)
            if match:
                self._maps.append(match.group('mapname').lower())

        self._maps.sort()



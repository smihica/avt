# -*- coding: utf-8 -*-

require 'socket'
require 'thread'
require 'rubygems'
require 'em-websocket'

## (1) ( browser -> ws -> pipe_w ) ----> (2) ( pipe_r -> socket -> uav ) |
##                                                                       |
## (4) ( browser <- ws <- pipe_r ) <---- (3) ( pipe_w <- socket <- uav ) v

## (1)
def run_browser_receiver(msg, ws, connections, b2m_w)
  puts 'received from browser "' + msg + '"'
  if /^V (.*)\r\n$/ =~ msg
    msg = "v " + kanji_to_koe_romaji($1).chomp + "\r\n"
    puts 'voice kanji to koe replaced "' + msg + '"'
  end
  b2m_w.puts msg
  ws.send msg # echo back to myself
  connections.each {|con|
    con.send(msg) unless con == ws # to other people
  }
end

## (4)
def run_browser_sender(ws, connections, m2b_r)

  loop do
    msg = ("" + m2b_r.gets)
    puts 'send to browser "' + msg + '"'
    ws.send msg
    connections.each {|con|
      con.send(msg) unless con == ws
    }
  end
end

def kanji_to_koe_romaji(k)
  s = IO.popen("./kanjikoe/Kanji2KoeCmd", "r+") {|io|
    io.puts k
    io.close_write
    io.gets
  }
  return s
end

## (2)
def run_uav_sender(s, b2m_r)

  until File.select([b2m_r], [], [], 0) == nil
    b2m_r.gets
  end

  loop do
    msg = ("" + b2m_r.gets)
    puts 'send to uav "' + msg + '"'
    s.puts msg
  end
end

## (3)
def run_uav_receiver(s, m2b_w)
  loop do
    msg = ("" + s.gets)
    puts 'received from uav "' + msg + '"'
    m2b_w.puts msg
  end
end

def main()
  # Process.daemon(nochdir=true) if ARGV[0] == "-D"
  sport = 10001
  wport = 51234
  argv0 = (ARGV[0] or "0").to_i
  sport = 10001 + argv0
  wport = 51234 + argv0
  puts("server  port: ", sport)
  puts("websock port: ", wport)

  server = TCPServer.open(sport)
  b2m_r, b2m_w = IO.pipe
  m2b_r, m2b_w = IO.pipe
  p [ b2m_r, b2m_w, m2b_r, m2b_w ]

  Thread.start() do
    connections = Array.new
    EventMachine::WebSocket.start(:host => "0.0.0.0", :port => wport) do |ws|
      ws.onopen {
        ws.send "accepted browser"
        connections.push(ws) unless connections.index(ws)
        Thread.start() do
          run_browser_sender(ws, connections, m2b_r)
        end
      }
      ws.onmessage { |msg|
        run_browser_receiver(msg, ws, connections, b2m_w)
      }
      ws.onclose { puts "browser going to close." }
    end
  end

  loop do
    Thread.start(server.accept) do |s|
      command = s.gets.chomp
      if command.match(/^BOT$/)
        puts 'uav accepted.'
        puts 'thread for (2) standby.'
        Thread.start() do
          puts 'thread for (3) standby.'
          run_uav_receiver(s, m2b_w)
        end
        run_uav_sender(s, b2m_r)
      end
      s.close
    end
  end
end

main()

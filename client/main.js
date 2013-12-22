// util
function clip(min, max, d) {
  if (d < min) return min;
  if (max < d) return max;
  return d;
}

function map_ratio(r, min, max, f) {
  return (min + ((max - min) * (f ? f(r) : r)))|0;
}

function quad_f(r) {
  r = ((r * 2.0) - 1.0);
  return ((((r < 0) ? -1 : 1) * (r*r)) + 1.0) / 2.0;
}

function strip (str) {
  return str.replace(/^\s*/, "").replace(/\s*$/, "");
}

function diffp(diff, num1, num2) {
  return (diff < Math.abs(num1 - num2));
}

// code

var DEFAULT = {
  accel:     127,
  direction: 127,
  panh:      127,
  panv:      127,
  batt_sw1:  0,
  batt_sw2:  0
};

var CAR_SETTINGS = {
  "0": {
    range: {
      accel:     [0,  255],
      direction: [0,  255],
      panh:      [0,  255],
      panv:      [56, 255]
    }
  },
  "1": {
    range: {
      accel:     [0,  255],
      direction: [0,  255],
      panh:      [0,  240],
      panv:      [46, 234],
    }
  }
};


// voice recgnition.
var Recognition = classify("Recognition", {
  property: {
    // active: null,
    parent: null,
    recognition: null
  },
  method: {
    init: function(uav) {
      this.parent = uav;
    },
    create_recognition: function() {
      var self = this;
      var recognition = new webkitSpeechRecognition();
      recognition.lang = "ja-JP";
      recognition.interimResults = false; // 中間結果なし
      recognition.continuous = true;      // 連続認識
      recognition.maxAlternatives = 3;    // 選択候補数(max)
      recognition.onsoundstart = function(){
        console.log("認識中");
      };
      recognition.onnomatch = function(){
        console.log("認識出来ません");
      };
      recognition.onerror= function(){
        console.log("エラー");
      };
      recognition.onsoundend = function(){
        console.log("停止中");
      };
      recognition.onresult = function(event) {
        var res = [];
        if (typeof(event.results) == 'undefined') {
          alert('最新のGoogle Chromeのみで動作します');
          recognition.onend = null;
          recognition.stop();
          return;
        }
        var results = event.results;
        for (var i = event.resultIndex, l = results.length; i < l; i++){
          if(results[i].isFinal) {
            res.push({
              primary:      results[i][0].transcript.replace(/^\s+/, ''),
              alternatives: Array.prototype.slice.call(results[i], 1).map(function(x){ return x.transcript.replace(/^\s+/, ''); })
            });
          }
        }
        if (0 < res.length) self.onreceived(res);
      };
      return recognition;
    },

    onreceived: function(res) {
      console.log("認識：" + res[0].primary);
      var txt = res[0].primary;
      this.parent.speak(txt, true);
      $("#speak").text( " << " + txt + '.' );
    },
    start: function() {
      if (this.recognition) { this.stop(); }
      var rec = this.create_recognition();
      rec.start();
      this.recognition = rec;
    },
    stop: function() {
      if (this.recognition) {
        this.recognition.stop();
        this.recognition = null;
      }
    }
  }
});

// control connection
var UAVConnection = classify("UAVConnection", {
  property: {
    ws: null
  },
  method: {
    init: function(id) {
      var self = this;
      var port = "51234";
      if (id === "1") { port = "51235"; }
      this.ws = new WebSocket("ws://uav.local:" + port);

      this.ws.onmessage = function(evt) {
        var data = strip(evt.data);
        var mat = data.match(/^b (\d\d\d\d)(\d\d\d\d)/);
        if (mat) {
          var b1 = RegExp.$1;
          var b2 = RegExp.$2;
          $("#batt").html(" Battery1: " + b1.replace(/0/g, "_").replace(/1/g, "*") +
                          " Battery2: " + b2.replace(/0/g, "_").replace(/1/g, "*"));
        }
      };

      this.ws.onclose = function() {
        console.log("closed connection.");
        self.ws = null;
      };

      this.ws.onopen = function() {
        self.ws.send("reset");
      };

    },
    send: function(data) {
      if (this.ws) this.ws.send(data);
    }
  }
});

var UAV = classify("UAV", {
  property: {
    con: null,
    data: null,
    recognition: null,
    targetCarID: '0'
  },
  method: {
    init: function(id) {
      var self = this;
      this.con = new UAVConnection(id);
      this.data = {
        accel:     DEFAULT.accel,
        direction: DEFAULT.direction,
        panh:      DEFAULT.panh,
        panv:      DEFAULT.panv,
        batt_sw1:  DEFAULT.batt_sw1,
        batt_sw2:  DEFAULT.batt_sw2
      };
      this.recognition = new Recognition(this);
      if (id) this.targetCarID = id;
      $("#car_select").change(function(e) {
        self.targetCarID = $(this).val();
      });
      $("#speak_input").keyup(function(e){
        if (e.keyCode == 13) {
          var string = $(this).val();
          console.log(string);
          self.speak(string, true);
          $("#speak").text( " << " + string + '.' );
          $(this).css({background:"#F66",color:"white"});
          var iself = this;
          setTimeout(function() {
            $(iself).css({background:"#FFF",color:"black"});
            $(iself).val("");
          }, 200);
        }
      });
    },
    setStatus: function (status) {
      var range = CAR_SETTINGS[this.targetCarID].range;
      var stick = status.stick;
      var d = this.data;
      d.accel     = stick.left.v.functionApply(quad_f).map(range.accel[0], range.accel[1]).get();
      d.direction = stick.left.h.invert().map(range.direction[0], range.direction[1]).get();
      d.panh      = stick.right.h.invert().map(range.panh[0], range.panh[1]).get();
      d.panv      = stick.right.v.invert().map(range.panv[0], range.panv[1]).get();
      d.batt_sw1  = d.batt_sw2 = (status.keys['start'] == 0 ? 0 : 1);

      if (status.keys['circle'] != 0) {
        this.recognition.start();
      } else {
        this.recognition.stop();
      }

      this.applyStatus();
      this.displayStatus(status, speak);
    },
    applyStatus: function() {
      this.con.send('w ' + this.data.accel +
                    ' '  + this.data.direction +
                    ' '  + this.data.panh +
                    ' '  + this.data.panv +
                    ' '  + this.data.batt_sw1 +
                    ' '  + this.data.batt_sw2);
    },
    displayStatus: function (status) {
      $("#data").html(
        (" acc: "      + this.data.accel +
         " dir: "      + this.data.direction +
         " panh: "     + this.data.panh +
         " panv: "     + this.data.panv +
         " batt_sw1: " + this.data.batt_sw1 +
         " batt_sw2: " + this.data.batt_sw2));
    },
    speak: function (str, kanji_p) {
      var header = 'v ';
      if (kanji_p) header = 'V ';
      this.con.send(header + str + '\r\n');
    }
  }
});

var GameController = classify("GameController", {
  static: {
    keyNames: [
      "cross", "circle", "square", "triangle", "L1", "R1", "L2", "R2",
      "select", "start", "stickL", "stickR", "up", "down", "left", "right", "PS"
    ],
    Stick: classify("GameControllerStick", {
      property: {
        source: null,
        filters: []
      },
      method: {
        init: function(source) {
          this.source = source;
        },
        get: function() {
          var data = this.source();
          for (var i = 0, l = this.filters.length; i < l; i++) {
            var f = this.filters[i];
            switch(f[0]) {
            case 'invert':
              data = 1.0 - data;
              break;
            case 'map':
              data = map_ratio(data, f[1], f[2]);
              break;
            case 'functionApply':
              data = f[1](data);
              break;
            }
          }
          delete this.filters;
          this.filters = [];
          return data;
        },
        invert: function() {
          this.filters.push(['invert']);
          return this;
        },
        map:    function(min, max) {
          this.filters.push(['map', min, max]);
          return this;
        },
        functionApply: function(fn) {
          this.filters.push(['functionApply', fn]);
          return this;
        }
      }
    })
  },
  property: {
    onStatusChanged: function(){},
    accuracy: 0.001,
    loopFlog: null,
    targetController: 0,
    controllerStatus: {
      stickRaw: {
        left:  {
          v: 0.5, h: 0.5
        },
        right: {
          v: 0.5, h: 0.5
        }
      },
      stick: {
        left:  { v: null, h: null },
        right: { v: null, h: null }
      },
      keys: {
        triangle: 0, // 0 unpressing 1 onTouch 2 pressing 3 onTouchOff
        square:   0,
        circle:   0,
        cross:    0,
        start:    0,
        select:   0,
        stickL:   0,
        stickR:   0,
        up:       0,
        left:     0,
        right:    0,
        down:     0,
        R1:       0,
        R2:       0,
        L1:       0,
        L2:       0,
        PS:       0
      }
    },
  },
  method: {
    init: function(accuracy, onStatusChanged) {
      if (!navigator.webkitGetGamepads) {
        throw new Error("Gamepad is not supported. You must browse this site by firefox or chrome.");
      }

      var stickRaw  = this.controllerStatus.stickRaw;
      var stick     = this.controllerStatus.stick;
      stick.left.v  = new GameController.Stick(function(){return stickRaw.left.v;});
      stick.left.h  = new GameController.Stick(function(){return stickRaw.left.h;});
      stick.right.v = new GameController.Stick(function(){return stickRaw.right.v;});
      stick.right.h = new GameController.Stick(function(){return stickRaw.right.h;});

      if (accuracy)        this.accuracy = 1.0 / accuracy;
      if (onStatusChanged) this.onStatusChanged = onStatusChanged;

      this.targetController = (+$("#target_controller").val());
      console.log(this.targetController);

      this.startLoop();
    },
    detectTarget: function() {
    },
    makeSnapshot: function() {
      var stickRaw = this.controllerStatus.stickRaw;
      var keys = this.controllerStatus.keys;
      var keyNames = GameController.keyNames;
      delete this.snapshot;
      this.snapshot = [
        stickRaw.left.h,
        stickRaw.left.v,
        stickRaw.right.h,
        stickRaw.right.v
      ];
      for (var i = 0, l = keyNames.length; i < l; i++) {
        this.snapshot[i+4] = keys[keyNames[i]];
      }
    },
    compareStatus: function() {
      var stickRaw = this.controllerStatus.stickRaw;
      var keys = this.controllerStatus.keys;
      var keyNames = GameController.keyNames;
      var snapshot = this.snapshot;
      if (diffp(this.accuracy, snapshot[0], stickRaw.left.h) ||
          diffp(this.accuracy, snapshot[1], stickRaw.left.v) ||
          diffp(this.accuracy, snapshot[2], stickRaw.right.h) ||
          diffp(this.accuracy, snapshot[3], stickRaw.right.v)) return true;
      for (var i = 0, l = keyNames.length; i < l; i++) {
        if (snapshot[i+4] != keys[keyNames[i]]) return true;
      }
      return false;
    },
    startLoop: function() {
      var self = this;
      var status = this.controllerStatus;
      var stickRaw = this.controllerStatus.stickRaw;
      var keys = this.controllerStatus.keys;
      this.loopFlag = setInterval(function() {
        var gamepad = navigator.webkitGetGamepads()[self.targetController];
        if (gamepad) {

          self.makeSnapshot();

          stickRaw.left.h  = ((gamepad.axes[0] + 1.0) / 2.0);
          stickRaw.left.v  = ((gamepad.axes[1] + 1.0) / 2.0);
          stickRaw.right.h = ((gamepad.axes[2] + 1.0) / 2.0);
          stickRaw.right.v = ((gamepad.axes[3] + 1.0) / 2.0);
          var buttons  = gamepad.buttons;
          var keyNames = GameController.keyNames;
          for (var i = 0, l = Math.min(buttons.length, keyNames.length); i < l; i++) {
            var now  = buttons[i];
            var name = keyNames[i];
            var prev = keys[name], next = 0;
            if (now == 0) {
              if (prev == 1 || prev == 2) {
                next = 3;
              }
            } else {
              if (prev == 0 || prev == 3) {
                next = 1;
              } else {
                next = 2;
              }
            }
            keys[name] = next;
          }

          if (self.compareStatus()) {
            self.onStatusChanged(status);
          }
        }
      }, 50);
    },
    stopLoop: function() {
      clearInterval(this.loopFlag);
    },
    setOnStatusChanged: function(fn) {
      this.onStatusChanged = fn;
    }
  },
});

$(function() {
  var uav = new UAV($("#car_select").val());
  var controller = new GameController(80, function(status) {
    var html = '', keyNames = GameController.keyNames;
    for (var i = 0, l = keyNames.length; i < l; i++) {
      var n = keyNames[i];
      var ks = status.keys[n];
      if (ks != 0) {
        html += '<span class="keys ' + ((ks == 1) ? 'onpress' : (ks == 2) ? 'hold' : 'onrelease') + '">' + n + '</span> ';
      }
    }
    $("#pad").html(html);
    return uav.setStatus(status);
  });

  var audio_url = $("#audio_feed").attr('src');
  $f("audio_feed", "flowplayer-3.2.15.swf", {
    plugins: {
      controls: {
        fullscreen: false,
        height: 30,
        autoHide: true,
        play: false,
      }
    },
    clip: {
      autoPlay: true,
      autoBuffering: false,
      bufferLength: 0,
      url: audio_url
    }
  });

});

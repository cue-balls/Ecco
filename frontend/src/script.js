import { Chess } from 'https://cdn.jsdelivr.net/npm/chess.js@1.0.0-beta.6/+esm';

const white_button = document.getElementById("play_w");
const black_button = document.getElementById("play_b");
const textbox = document.getElementById("fen");

const config = {
  position: 'start',
  pieceTheme: 'https://chessboardjs.com/img/chesspieces/wikipedia/{piece}.png',
  draggable: true,
  snapSpeed: 0,
  moveSpeed: 250,
  onDragStart: onDragStart,
  onDrop: onDrop,
  onSnapEnd: onSnapEnd,
};

const chess = new Chess();

var board;
var player_color = "";
var turn = "white";

const delay = (ms) => new Promise(resolve => setTimeout(resolve, ms));

function onDragStart (source, piece, position) {
  if (chess.isGameOver()) {
    return false;
  }

  if ((player_color == "white" && piece.search(/^w/) === -1) ||
      (player_color == "black" && piece.search(/^b/) === -1) ||
    player_color != turn) {
    return false;
  }
}

function onSnapEnd (piece, square, position, orientation) {
  board.position(chess.fen(), false);
}

function onDrop (source, target, piece, newPos, oldPos, orientation) {
  const verboseMoves = chess.moves({ verbose: true });
  const lanMovesList = verboseMoves.map(move => move.lan);

  var validate = false;
  let player_move;

  lanMovesList.forEach((move) => {
    if (move == source + target || move == source + target + "q") {
      validate = true;
      player_move = chess.move({from: source, to: target, promotion: "q"});
      board.position(chess.fen(), false);

      if (turn == "white")
      {
        turn = "black";
      }
      else
      {
        turn = "white";
      }

    }
  });

  if (!validate) {
    return "snapback";
  }

  var pos = chess.fen();
  if (player_move && player_move.flags.includes('b')) {
        const file = player_move.to[0]; 
        const epRank = player_move.color === 'w' ? '3' : '6'; 
        const targetSquare = file + epRank;
        const fenParts = pos.split(' ');
        fenParts[3] = targetSquare; 
        
        pos = fenParts.join(' ');
    }


  
  updateBoard().then(() => {
    if (!chess.isGameOver())
    {
      playEngineMove(pos); 
    }
  });
}

async function updateBoard() {
  await delay(500);
}

white_button.addEventListener('click', async () => {
    player_color = "white";
    board = Chessboard("myBoard", config);
    board.orientation("white");
    white_button.style.display = "none";
    black_button.style.display = "none";

    var pos = chess.fen();
    if (textbox.value != "")
    {
      chess.load(textbox.value);
      board.position(textbox.value);
      pos = textbox.value;
    }

    if (!chess.isGameOver())
    {
      if (chess.turn() == "w")
      {
        turn = "white";
      }
      else
      {
        turn = "black";
      }

      if (turn == "black")
      {
        playEngineMove(pos);
        await delay(500);
        board.position(chess.fen());
      }
    }
});

black_button.addEventListener('click', async () => {
    player_color = "black";
    board = Chessboard("myBoard", config);
    board.orientation("black");
    white_button.style.display = "none";
    black_button.style.display = "none";

    var pos = chess.fen();
    if (textbox.value != "")
    {
      chess.load(textbox.value);
      board.position(textbox.value);
      pos = textbox.value;
    }

    if (!chess.isGameOver())
    {
      if (chess.turn() == "w")
      {
        turn = "white";
      }
      else
      {
        turn = "black";
      }
    }

    if (turn == "white")
    {
      await delay(500);
      playEngineMove(pos);
      await delay(500);
      board.position(chess.fen());
    }

    
});


async function fetchEngineMove(pos) {
  let data;
  try {
        const response = await fetch("http://127.0.0.1:8080/data", {
          method: "POST",
          headers: {
            "Content-Type": "text/plain"
          },

          body: pos
        });
        
        if (!response.ok) {
            throw new Error(`HTTP error! Status: ${response.status}`);
        }

        data = await response.json();
        console.log('Data retrieved from C++:', data);
  } catch (error) {
        console.error('Error fetching data:', error);
  }


  return data.move;
}

async function parseMove() {
  const move = await fetchEngineMove(pos);
  return move.toString();
}

function playEngineMove(pos) {
  let engineMove;
  let start;
  let end;
  let promote;

  fetchEngineMove(pos).then((move) => {
    engineMove = move.toString();
    console.log(engineMove);
    start = engineMove.substring(0, 2);
    end = engineMove.substring(2, 4);
    promote = 'q';

    if (engineMove.length == 5) {
      promote = engineMove[4];
    }

    chess.move({from: start, to: end, promotion: promote});

    if (turn == "white")
    {
      turn = "black";
    }
    else
    {
      turn = "white";
    }

    board.position(chess.fen());  

  });
}
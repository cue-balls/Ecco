import { Chess } from 'https://cdn.jsdelivr.net/npm/chess.js@1.0.0-beta.6/+esm';

const white_button = document.getElementById("play_w");
const black_button = document.getElementById("play_b");

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

  lanMovesList.forEach((move) => {
    if (move == source + target || move == source + target + "q") {
      validate = true;
      chess.move({from: source, to: target, promotion: "q"});
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


  
  updateBoard().then(() => {
    if (!chess.isGameOver())
    {
      chess.move(chess.moves()[0]);

      if (turn == "white")
      {
        turn = "black";
      }
      else
      {
        turn = "white";
      }
    }
    
    board.position(chess.fen());  
  });
}

async function updateBoard() {
  await delay(500);
}

white_button.addEventListener('click', () => {
    player_color = "white";
    board = Chessboard("myBoard", config);
    board.orientation("white");
    white_button.style.display = "none";
    black_button.style.display = "none";
});

black_button.addEventListener('click', async () => {
    player_color = "black";
    board = Chessboard("myBoard", config);
    board.orientation("black");
    white_button.style.display = "none";
    black_button.style.display = "none";

    chess.move(chess.moves()[0]);
    await delay(500);
    board.position(chess.fen());
    turn = "black";
});



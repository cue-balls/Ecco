import { Chess } from 'https://cdn.jsdelivr.net/npm/chess.js@1.0.0-beta.6/+esm';

const white_button = document.getElementById("play_w");
const black_button = document.getElementById("play_b");

const config = {
  position: 'start',
  pieceTheme: 'https://chessboardjs.com/img/chesspieces/wikipedia/{piece}.png',
  draggable: true,
  snapSpeed: 50,
  moveSpeed: 150,
  onDragStart: onDragStart,
  onDrop: onDrop,
};

const chess = new Chess();

var board;
var player_color = "";
var turn = "white";

const delay = (ms) => new Promise(resolve => setTimeout(resolve, ms));

function onDragStart (source, piece, position) {
  //console.log(player_color);
  //console.log(turn);
  if ((player_color == "white" && piece.search(/^w/) === -1) ||
      (player_color == "black" && piece.search(/^b/) === -1) ||
    player_color != turn) {
    return false;
  }
}

function onDrop (source, target, piece, newPos, oldPos, orientation) {
  //console.log("drop");
  //console.log(chess.fen());

  const verboseMoves = chess.moves({ verbose: true });
  const lanMovesList = verboseMoves.map(move => move.lan);
  //console.log(source + target);

  var validate = false;

  lanMovesList.forEach((move) => {
    //console.log(move == source + target);
    if (move == source + target) {
      validate = true;
      chess.move({from: source, to: target});
      //board.position(chess.fen());

      if (turn == "white")
      {
        //console.log("w")
        turn = "black";
      }
      else
      {
        //console.log("b");
        turn = "white";
      }

    }
  });

  //console.log(validate);
  if (!validate) {
    return "snapback";
  }

  board.position(chess.fen());
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



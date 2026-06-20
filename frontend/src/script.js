import { Chess } from 'https://cdn.jsdelivr.net/npm/chess.js@1.0.0-beta.6/+esm';

const white_button = document.getElementById("play_w");
const black_button = document.getElementById("play_b");

const config = {
  position: 'start',
  pieceTheme: 'https://chessboardjs.com/img/chesspieces/wikipedia/{piece}.png',
  draggable: true,
  onDragStart: onDragStart,
  onDrop: onDrop,
};

var board
var player_color = "";
var turn = "white";

function onDragStart (source, piece, position) {
  if ((player_color == "white" && piece.search(/^w/) === -1) ||
      (player_color == "black" && piece.search(/^b/) === -1) ||
    player_color != turn) {
    return false;
  }
}

function onDrop (source, target, piece, newPos, oldPos, orientation) {
  const verboseMoves = chess.moves({ verbose: true });
  const lanMovesList = verboseMoves.map(move => move.lan);
  //console.log(source + target);

  var validate = false;

  lanMovesList.forEach((move) => {
    //console.log(move);
    if (move == source + target) {
      validate = true;
    }
  });

  if (!validate) {
    return "snapback";
  }
}



white_button.addEventListener('click', () => {
    player_color = "white";
    board = Chessboard("myBoard", config);
    board.orientation("white");
    white_button.style.display = "none";
    black_button.style.display = "none";
});

black_button.addEventListener('click', () => {
    player_color = "black";
    board = Chessboard("myBoard", config);
    board.orientation("black");
    white_button.style.display = "none";
    black_button.style.display = "none";
});


const chess = new Chess();
const moves = chess.moves();
console.log(moves);


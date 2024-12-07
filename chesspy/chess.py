from typing import Dict, Union, List, Tuple

board_dtype = Dict[Tuple[int,int], Union[str,None]]

# How the move history is recorded:
# (...) indicates this might not be present in the history element

# { "player":player, 
#   "start":start_square, 
#   "end":end_square, 
#   "end_prev":previous_piece, 
#   ("broke_castle_short":None), 
#   ("broke_castle_long":None) }

# { "player":player, 
#   "special":"castle_short"
#   "end_prev":previous_piece, 
#   "broke_castle_short":None, 
#   ("broke_castle_long":None) }

# { "player":player, 
#   "special":"castle_long"
#   "end_prev":previous_piece, 
#   ("broke_castle_short":None), 
#   "broke_castle_long":None }

deltas = {}
deltas["b"] = [(-1,-1),(1,-1),(-1,1),(1,1)]
deltas["r"] = [(-1,0),(1,0),(0,-1),(0,1)]
deltas["q"] = deltas["b"] + deltas["r"]
deltas["k"] = deltas["q"]


def create_board():
    return {(i,j):None for j in range(8) for i in range(8)}

def reset_board(brd):
    for i in range(8):
        brd[i,1] = "pw"
        brd[i,6] = "pb"
    for j,p in zip([0,7], ["w","b"]):
        brd[0,j] = "r"+p
        brd[7,j] = "r"+p
        brd[1,j] = "n"+p
        brd[6,j] = "n"+p
        brd[2,j] = "b"+p
        brd[5,j] = "b"+p
        brd[3,j] = "q"+p
        brd[4,j] = "k"+p


def i2s(i,j):
    return chr(i+ord("a")) + str(j+1)

def s2i(idx):
    return ord(idx[0]) - ord("a"), int(idx[1]) - 1

def I2S(arr):
    return [i2s(i,j) for i,j in arr]

def S2I(arr):
    return [s2i(idx) for idx in arr]


def raycast(brd : board_dtype, start : Tuple[int,int], deltas : List[Tuple[int,int]]) -> List[Tuple[int,int]]:

    i0, j0 = start
    assert(0<=i0<8 and 0<=j0<8)
    squares = []

    for di,dj in deltas:
        i = i0 + di
        j = j0 + dj
        while 0<=i<8 and 0<=j<8:
            squares.append((i, j))
            if brd[i,j] is None:
                i += di
                j += dj
            else:
                break

    return squares


def print_board(brd : board_dtype):
    for i in range(8):
        print(" ".join([brd[i,j] if brd[i,j] else ("::" if (i+j)%2==0 else "  ") for j in range(8)]))

# determines for each square whether a pawn can be pushed to that square
# doesn't require the square to be empty, although for double push it requires the rank 3 or rank 6 is empty
def get_pawn_pushes(brd : board_dtype):
    pawn_pushes = {(i,j): {"w":[], "b":[]} for j in range(8) for i in range(8)}
    for i in range(8):
        for j in range(8):
            if brd[i,j] and brd[i,j][0] == "p":
                if brd[i,j][1] == "w" and j+1 < 8:
                    pawn_pushes[i,j+1]["w"].append((i,j))
                    if j == 1 and brd[i,2] == None: # Make sure that the intermediate square is empty for double push
                        pawn_pushes[i,3]["w"].append((i,j))
                if brd[i,j][1] == "b" and 0<=j-1:
                    pawn_pushes[i,j-1]["b"].append((i,j))
                    if j == 6 and brd[i,5] == None: # Make sure that the intermediate square is empty for double push
                        pawn_pushes[i,4]["b"].append((i,j))
    return pawn_pushes

# determines for each square which pieces are covering that square (i.e. would be able to take something on that square)
def get_covers(brd : board_dtype):
    covers = {(i,j): {"w":[], "b":[]} for j in range(8) for i in range(8)}
    for i in range(8):
        for j in range(8):
            if brd[i,j] is not None:
                piece = brd[i,j][0]
                owner = brd[i,j][1]
    
                if piece == "p":
                    
                    deltas = [(-1,1),(1,1)] if owner == "w" else [(-1,-1), (1,-1)]
                    for di,dj in deltas:
                        if 0<=i+di<8 and 0<=j+dj<8:
                            covers[i+di,j+dj][owner].append((i,j))
    
                elif piece == "n":
                    
                    deltas = [(-2,-1),(-2,1),(2,-1),(2,1),(-1,-2),(-1,2),(1,-2),(1,2)]
                    for di,dj in deltas:
                        if 0<=i+di<8 and 0<=j+dj<8:
                            covers[i+di,j+dj][owner].append((i,j))
                
                elif piece == "k":
                    
                    deltas = [(-1,0),(1,0),(0,-1),(0,1),(-1,-1),(1,-1),(-1,1),(1,1)]
                    for di,dj in deltas:
                        if 0<=i+di<8 and 0<=j+dj<8:
                            covers[i+di,j+dj][owner].append((i,j))
    
                elif piece == "b":
    
                    deltas = [(-1,-1),(1,-1),(-1,1),(1,1)]
                    squares = raycast(brd, (i,j), deltas)
                    for x in squares:
                        covers[x][owner].append((i,j))
                    
                elif piece == "r":
    
                    deltas = [(-1,0),(1,0),(0,-1),(0,1)]
                    squares = raycast(brd, (i,j), deltas)
                    for x in squares:
                        covers[x][owner].append((i,j))
    
                elif piece == "q":
    
                    deltas = [(-1,0),(1,0),(0,-1),(0,1),(-1,-1),(1,-1),(-1,1),(1,1)]
                    squares = raycast(brd, (i,j), deltas)
                    for x in squares:
                        covers[x][owner].append((i,j))

    return covers


def get_available_moves(brd : board_dtype, playing : str, can_castle=None, enpassant=None):
    assert(playing in ["w", "b"])

    enemy = "b" if playing == "w" else "w"
    covers = get_covers(brd)
    pawn_pushes = get_pawn_pushes(brd)
    
    # determine if player is in check
    checking_pieces = []
    pinned_pieces = []
    pinned_pieces_span = {}
    pinned_pieces_threat = {}

    # get position of the king (which will always exist)
    kpos = [(i,j) for j in range(8) for i in range(8) if brd[i,j] == "k"+playing][0]
    
    # store any enemy pieces that are covering the king's square
    checking_pieces = covers[kpos][enemy]
    
    # cast rays from the king to get bishops, rooks or queens that are xraying the king either directly or through a pin
    # store the intermediate spanning squares along the ray between the king and potential enemy piece
    # if there's a unobstructed ray then this will be used for checking blocking moves
    # if the ray's is unobstructed then the enemy piece will have already been identified as a checking piece as it will cover the king's square
    

    threat_pieces = ["b","r","q"]
    
    king_rays = {}
    threat_180_neighbours = []
    
    for d in deltas["k"]:
        di,dj = d
        
        king_rays[d] = {"friendly":[], "span":[], "threat":None, "180_neighbour":None}

        # if there is a bishop, rook or queen checking the king on an unobstructed ray
        # the king will not be able to move the direction 180deg to the ray as it will still be in check
        # therefore store this square as a member of the ray to be used to prevent the king from moving to this square
        if 0 <= kpos[0]-di < 8 and 0 <= kpos[1]-dj < 8:
            king_rays[d]["180_neighbour"] = (kpos[0] - di, kpos[1] - dj)
        
        i = kpos[0] + di
        j = kpos[1] + dj
        
        # increment along the direction of the ray from the first square in the span (not including the king)
        # if there are any friendly pieces in the span then make note of them. If an enemy piece is hit then
        # if this is a bishop, rook or queen then depending the delta, this will be a threat to the king. Make note of this in ray.
        # it will be checking the king if there are no friendly pieces present but this will already be realised
        # by the covers on the king's square. Stop the ray cast once including the first enemy piece as there is no need to go any further.
        while 0<=i<8 and 0<=j<8:
            if brd[i,j] is not None:
                if brd[i,j][1] == playing:
                    king_rays[d]["friendly"].append((i,j))
                elif brd[i,j][1] == enemy:
                    if brd[i,j][0] in threat_pieces and d in deltas[brd[i,j][0]]:
                        king_rays[d]["threat"] = (i,j)
                    break
                
            king_rays[d]["span"].append((i,j))
            i += di
            j += dj

        # if there is a threat checking the king along this ray then make note of the 180_neighbour
        # as the king cannot move here
        if king_rays[d]["threat"] is not None and len(king_rays[d]["friendly"]) == 0:
            threat_180_neighbours.append(king_rays[d]["180_neighbour"])
        
        # if there is a threat along this ray and there is only one friendly piece in the span of the ray
        # between the king and threat, then this piece is pinned and can only move within the span of the ray
        if king_rays[d]["threat"] is not None and len(king_rays[d]["friendly"]) == 1:
            pinned_piece = king_rays[d]["friendly"][0]
            pinned_pieces.append(pinned_piece)
            pinned_pieces_span[pinned_piece] = king_rays[d]["span"]
            pinned_pieces_threat[pinned_piece] = king_rays[d]["threat"]
    
    moves = []
    
    if len(checking_pieces) > 0:
        # KING IS IN CHECK
        in_check = True

        # if 1 piece is checking the king then it can be taken
        # and if it's a bishop, rook or queen checking the king from a distance it can be blocked
        if len(checking_pieces) == 1:
            threat = checking_pieces[0]
            
            # BLOCKING MOVES
            # **************************
            # get the spans between the king and checking piece if the checking piece is bishop, rook or queen
            # these spans are the squares that can be used to block the direct check xray by one of the playing's pieces
            # note that the span will contain no pieces as it's a direct xray
            spans = [ x["span"] for x in king_rays.values() if x["threat"] == threat ]
            if len(spans) > 0:
                for x in spans[0]:
                    # look at covers on the span squares by all pieces except for king and pawns
                    # as the pawn cover is to do with taking and not pushing and the king cannot move within the xray span
                    for y in covers[x][playing]:
                        if brd[y][0] != "k" and brd[y][0] != "p":
                            moves.append({"start":y, "end":x})
                    # now look at pawn pushes into the span squares
                    for y in pawn_pushes[x][playing]:
                        moves.append({"start":y, "end":x})
        
                    
            # TAKING THREAT PIECE
            # **************************
            # if the threat's square is covered by the playing then this piece can be taken without restriction if it's not the 
            # playing's king. If it is the playing's king then check that the threat's square is not covered by the enemy
            threat_covered = (len(covers[threat][enemy]) > 0)
        
            for x in covers[threat][playing]:
                xp = brd[x]
                if xp[0] != "k":
                    moves.append({"start":x, "end":threat})
                elif xp[0] == "k" and threat_covered == False:
                    moves.append({"start":x, "end":threat})
            
            # handle the case of an enpassant execute that takes the checking piece (if this piece is the double pushed enemy pawn)
            # there is no other case where en passant would be used to get out of check
            '''
            if enpassant and threat == enpassant["victim"]:
                for x in enpassant["attackers"]:
                    moves.append({"start":x, "end":enpassant["attacker_end"], "special":"enpassant", "enpassant_execute":{"victim":enpassant["victim"]}})
            '''

        # MOVING KING AWAY
        # **************************
        for di,dj in deltas["k"]:
            i = kpos[0] + di
            j = kpos[1] + dj

            # check that this is a valid square, that it's not covered by enemy and that it doesn't contain friendly piece
            # and is not the 180_neighbour of the checking piece(s)
            if (0<=i<8 and 0<=j<8) and (len(covers[i,j][enemy])==0) and ((brd[i,j][1] if brd[i,j] else enemy) == enemy) and ((i,j) not in threat_180_neighbours):
                moves.append({"start":kpos, "end":(i,j)})
    
    
    else:
        # KING IS NOT IN CHECK
        in_check = False

        for i in range(8):
            for j in range(8):
                for x in covers[i,j][playing]:
                    piece = brd[x][0]
                    valid_move = False
                    if piece == "p" and brd[i,j] and brd[i,j][1] == enemy:
                        valid_move = True
                    elif piece == "k" and (brd[i,j] is None or brd[i,j][1] == enemy) and len(covers[i,j][enemy]) == 0:
                        valid_move = True
                    elif piece in ["n", "b", "r", "q"] and (brd[i,j] is None or brd[i,j][1] == enemy):
                        valid_move = True
    
                    if valid_move:
                        # if the piece is pinned check that the move keeps it within its span
                        if x in pinned_pieces and ((i,j) in pinned_pieces_span[x] or (i,j) == pinned_pieces_threat[x]):     ## INCLUDE ENEMY PIECE IN THIS SPAN!!!
                            moves.append({"start":x, "end":(i,j)})
                        elif x not in pinned_pieces:
                            moves.append({"start":x, "end":(i,j)})

                for x in pawn_pushes[i,j][playing]:
                    if brd[i,j] is None:
                        # if the pawn is pinned check that the move keeps it in its span
                        if x in pinned_pieces and (i,j) in pinned_pieces_span[x]:
                            moves.append({"start":x, "end":(i,j)})
                        elif x not in pinned_pieces:
                            moves.append({"start":x, "end":(i,j)})

        # check for castling
        if can_castle:
            rank = 0 if playing=="w" else 7
            if can_castle[playing]["short"] and len(covers[5,rank][enemy])==0 and len(covers[6,rank][enemy])==0 and brd[5,rank]==None and brd[6,rank]==None:
                moves.append({"start":(4,rank), "end":(6,rank), "special":"castle_short"})
                print(playing, "can castle short")
            if can_castle[playing]["long"] and len(covers[3,rank][enemy])==0 and len(covers[2,rank][enemy])==0 and brd[3,rank]==None and brd[2,rank]==None and brd[1,rank]==None:
                moves.append({"start":(4,rank), "end":(2,rank), "special":"castle_long"})
                print(playing, "can castle long")
        
        # check if en passant can be made against the other player
        '''
        if enpassant:
            for x in enpassant["attackers"]:
                moves.append({"start":x, "end":enpassant["attacker_end"], "special":"enpassant", "enpassant_execute":{"victim":enpassant["victim"]}})
        '''

    # go through and check for triggering en passant
    '''
    for move in moves:
        if playing == "w" and brd[move["start"]] == "pw" and move["start"][1] == 1 and move["end"][1] == 3:
            enpassant_data = {"victim":move["end"], "attacker_end":(move["end"][0],2), "attackers":[]}
            i,j = move["end"]
            if i > 0 and brd[i-1,j] == "pb":
                enpassant_data["attackers"].append((i-1,j))
            if i < 7 and brd[i+1,j] == "pb":
                enpassant_data["attackers"].append((i+1,j))
            if len(enpassant_data["attackers"]) > 0:
                move["enpassant_trigger"] = enpassant_data
        elif playing == "b" and brd[move["start"]] == "pb" and move["start"][1] == 6 and move["end"][1] == 4:
            enpassant_data = {"victim":move["end"], "attacker_end":(move["end"][0],5), "attackers":[]}
            i,j = move["end"]
            if i > 0 and brd[i-1,j] == "pw":
                enpassant_data["attackers"].append((i-1,j))
            if i < 7 and brd[i+1,j] == "pw":
                enpassant_data["attackers"].append((i+1,j))
            if len(enpassant_data["attackers"]) > 0:
                move["enpassant_trigger"] = enpassant_data
    '''
    # go through moves and identify if any of them trigger draw
    
    return in_check, moves




class Chessboard:

    def __init__(self, playing=None, board=None):
        if board:
            self.board = board
        else:
            self.board = {(i,j):None for j in range(8) for i in range(8)}
            reset_board(self.board)

        self.history = []
        self.can_castle = {"w":{"long":True, "short":True}, "b":{"long":True, "short":True}}
        self.playing = playing if playing else "w"
        
        self.in_check, self.available_moves = get_available_moves(self.board, self.playing, self.can_castle)

    def print_status(self):
        print("")
        print(f"Playing: {self.playing}")
        print(f"In check: {self.in_check}")
        print(f"-- HISTORY --")
        for x in self.history:
            if "special" in x.keys():
                print(f"{x['player']} : {x['special']}")
            else:
                print(f'{x["player"]} : {x["start"]} -> {x["end"]} (replaced {x["end_prev"]})')
        print("")

    def copy_board(self, board):
        for i in range(8):
            for j in range(8):
                self.board[i,j] = board.board[i,j]
        
        self.playing = board.playing

        # copy the castling information
        for p in ["w","b"]:
            for t in ["short","long"]:
                self.can_castle[p][t] = board.can_castle[p][t]
        
        # calculate the available moves and whether the playing player is in check
        #print("Calculate moves for copied board")
        self.in_check, self.available_moves = get_available_moves(self.board, self.playing, self.can_castle)
        #print("Copying finished")


    def move(self, start, end):
        
        # get all the moves that match the start and end provided (there should only be 1 if any otherwise there is an issue)
        candidates = [x for x in self.available_moves if x["start"] == start and x["end"] == end]
        
        # get the rank of the playing player's backrank
        backrank = 0 if self.playing == "w" else 7

        if len(candidates) == 1:
            move = candidates[0]
            if "special" not in move.keys():
                # Non-special, normal move
                history_elem = {"player":self.playing, "start":start, "end":end, "end_prev":self.board[end]}

                # track whether player can still castle
                if (start == (4,backrank) or start == (0,backrank)) and self.can_castle[self.playing]["long"]:
                    # moving the king or moving the far rook
                    self.can_castle[self.playing]["long"] = False
                    history_elem["broke_castle_long"] = None
                    print(self.playing, "can no longer castle long")
                if (start == (4,backrank) or start == (7,backrank)) and self.can_castle[self.playing]["short"]:
                    # moving the king or moving the near rook
                    self.can_castle[self.playing]["short"] = False
                    history_elem["broke_castle_short"] = None
                    print(self.playing, "can no longer castle short")
                
                if self.board[start][0] == "p" and end[1] == 7 - backrank:
                    # pawn promotion
                    self.board[end] = "q" + self.playing
                else:
                    # normal move
                    self.board[end] = self.board[start]
                self.board[start] = None
                self.history.append(history_elem)

            else:
                if move["special"] == "castle_short":
                    history_elem = {"player":self.playing, "special":"castle_short"}
                    self.can_castle[self.playing]["short"] = False
                    history_elem["broke_castle_short"] = None
                    print(self.playing, "can no longer castle short")
                    if self.can_castle[self.playing]["long"]: 
                        self.can_castle[self.playing]["long"] = False
                        history_elem["broke_castle_long"] = None
                        print(self.playing, "can no longer castle long")
                    
                    self.board[4,backrank] = None
                    self.board[7,backrank] = None
                    self.board[6,backrank] = "k" + self.playing
                    self.board[5,backrank] = "r" + self.playing
                    self.history.append(history_elem)

                if move["special"] == "castle_long":
                    history_elem = {"player":self.playing, "special":"castle_long"}
                    self.can_castle[self.playing]["long"] = False
                    history_elem["broke_castle_long"] = None
                    print(self.playing, "can no longer castle long")
                    if self.can_castle[self.playing]["short"]: 
                        self.can_castle[self.playing]["short"] = False
                        history_elem["broke_castle_short"] = None
                        print(self.playing, "can no longer castle short") 
                                       
                    self.board[4,backrank] = None
                    self.board[0,backrank] = None
                    self.board[2,backrank] = "k" + self.playing
                    self.board[3,backrank] = "r" + self.playing
                    self.history.append(history_elem)
                    
                elif move["special"] == "enpassant":
                    pass

        else:
            print("Could not find move")

        self.playing = "w" if self.playing == "b" else "b"
        self.in_check, self.available_moves = get_available_moves(self.board, self.playing, self.can_castle)


    def get_king_pos(self, player):
        candidates = [(i,j) for j in range(8) for i in range(8) if self.board[i,j] == "k"+player]
        if len(candidates) > 0:
            return candidates[0]
        else:
            return None

    def undo_move(self):
        if len(self.history) > 0:
            move = self.history[-1]
            if "special" not in move.keys():
                self.board[move["start"]] = self.board[move["end"]]
                self.board[move["end"]] = move["end_prev"]
                other_player = "w" if self.playing == "b" else "b"

                if "broke_castle_long" in move.keys():
                    self.can_castle[other_player]["long"] = True
                    print(other_player,"can castle long again")
                if "broke_castle_short" in move.keys():
                    self.can_castle[other_player]["short"] = True
                    print(other_player,"can castle short again")
                del self.history[-1]
            else:
                if move["special"][:len("castle")] == "castle":
                    castle_type = move["special"][len("castle")+1:]
                    backrank = 0 if move["player"] == "w" else 7
                    other_player = "w" if self.playing == "b" else "b"
                    assert(move["player"] == other_player) # this is just for continuity. I expect the move player and other player to match up
                    print(f"Undoing castle, type {castle_type} for player {other_player}")
                    
                    if castle_type == "short":
                        self.board[4,backrank] = "k" + move["player"]
                        self.board[7,backrank] = "r" + move["player"]
                        self.board[5,backrank] = None
                        self.board[6,backrank] = None
                    elif castle_type == "long":
                        self.board[4,backrank] = "k" + move["player"]
                        self.board[0,backrank] = "r" + move["player"]
                        self.board[2,backrank] = None
                        self.board[3,backrank] = None

                    if "broke_castle_long" in move.keys():
                        self.can_castle[move["player"]]["long"] = True
                        print(move["player"],"can castle long again")
                    if "broke_castle_short" in move.keys():
                        self.can_castle[move["player"]]["short"] = True
                        print(move["player"],"can castle short again")
                    del self.history[-1]
                        
            self.playing = "w" if self.playing == "b" else "b"
            self.in_check, self.available_moves = get_available_moves(self.board, self.playing, self.can_castle) 
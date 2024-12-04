
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


def raycast(brd, start, deltas):
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




def show_squares(sqrs):
    for i in range(8):
        print("  ".join(["#" if (i,j) in sqrs else "." for j in range(8)]))


def print_board(brd):
    for i in range(8):
        print(" ".join([brd[i,j] if brd[i,j] else ("::" if (i+j)%2==0 else "  ") for j in range(8)]))


def get_pawn_pushes(brd):
    pawn_pushes = {(i,j): {"w":[], "b":[]} for j in range(8) for i in range(8)}
    for i in range(8):
        for j in range(8):
            if brd[i,j] is not None and brd[i,j][0] == "p":
                if brd[i,j][1] == "w" and j+1 < 8:
                    pawn_pushes[i,j+1]["w"].append((i,j))
                    if j == 1:
                        pawn_pushes[i,j+2]["w"].append((i,j))
                if brd[i,j][1] == "b" and 0<=j-1:
                    pawn_pushes[i,j-1]["b"].append((i,j))
                    if j == 6:
                        pawn_pushes[i,j-2]["b"].append((i,j))
    return pawn_pushes


def get_covers(brd):
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
                    for i1,j1 in squares:
                        covers[i1,j1][owner].append((i,j))
                    
                elif piece == "r":
    
                    deltas = [(-1,0),(1,0),(0,-1),(0,1)]
                    squares = raycast(brd, (i,j), deltas)
                    for i1,j1 in squares:
                        covers[i1,j1][owner].append((i,j))
    
                elif piece == "q":
    
                    deltas = [(-1,0),(1,0),(0,-1),(0,1),(-1,-1),(1,-1),(-1,1),(1,1)]
                    squares = raycast(brd, (i,j), deltas)
                    for i1,j1 in squares:
                        covers[i1,j1][owner].append((i,j))

    return covers


def get_available_moves(brd, playing, can_castle=None, enpassant=None):
    
    enemy = "b" if playing == "w" else "w"
    covers = get_covers(brd)
    pawn_pushes = get_pawn_pushes(brd)
    
    # determine if player is in check
    checking_pieces = []
    pinned_pieces = []
    pinned_pieces_spans = {}
    
    # get position of the king (which will always exist)
    kpos = [(i,j) for j in range(8) for i in range(8) if brd[i,j] == "k"+playing][0]
    
    # store any enemy pieces that are covering the king's square
    checking_pieces = covers[kpos][enemy]
    
    # cast rays from the king to get bishops, rooks or queens that are xraying the king either directly or through a pin
    # store the intermediate spanning squares along the ray between the king and potential enemy piece
    # if there's a unobstructed ray then this will be used for checking blocking moves
    king_deltas = [(-1,0),(1,0),(0,-1),(0,1),(-1,-1),(1,-1),(-1,1),(1,1)]
    threat_pieces = ["b","r","q"]
    king_rays = {}
    for di,dj in king_deltas:
        king_rays[di,dj] = {"friendly":[], "span":[], "enemy":None}
        i = kpos[0] + di
        j = kpos[1] + dj
        while 0<=i<8 and 0<=j<8:
            if brd[i,j] is not None:
                if brd[i,j][1] == playing:
                    king_rays[di,dj]["friendly"].append((i,j))
                elif brd[i,j][1] == enemy and brd[i,j][0] in threat_pieces:
                    king_rays[di,dj]["enemy"] = (i,j)
                    break
            king_rays[di,dj]["span"].append((i,j))
            i += di
            j += dj
        if king_rays[di,dj]["enemy"] is not None and len(king_rays[di,dj]["friendly"]) == 1:
            pinned_pieces.append(king_rays[di,dj]["friendly"][0])
            pinned_pieces_spans[king_rays[di,dj]["friendly"][0]] = king_rays[di,dj]["span"]
    
    moves = []
    
    # King is in check
    if len(checking_pieces) > 0:
        
        # if 1 piece is checking the king then it can be taken
        # and if it's a bishop, rook or queen checking the king from a distance it can be blocked
        if len(checking_pieces) == 1:
            threat = checking_pieces[0]
            
            # BLOCKING MOVES
            # **************************
            # get the spans between the king and checking piece if the checking piece is bishop, rook or queen
            # these spans are the squares that can be used to block the direct check xray by one of the playing's pieces
            # note that the span will contain no pieces as it's a direct xray
            spans = [ x["span"] for x in king_rays.values() if x["enemy"] == threat ]
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
            if enpassant and threat == enpassant["victim"]:
                for x in enpassant["attackers"]:
                    moves.append({"start":x, "end":enpassant["attacker_end"], "enpassant_execute":{"victim":enpassant["victim"]}})
        
        # MOVING KING AWAY
        # **************************
        for di,dj in king_deltas:
            i = kpos[0] + di
            j = kpos[1] + dj

            # check that this is a valid square, that it's not covered by enemy and that it doesn't contain friendly piece
            if 0<=i<8 and 0<=j<8 and len(covers[i,j][enemy])==0 and ((brd[i,j][1] if brd[i,j] else enemy) == enemy):
                moves.append({"start":kpos, "end":(i,j)})
    
    # King is not in check
    else:
        
        for i in range(8):
            for j in range(8):
                for x in covers[i,j][playing]:
                    piece = brd[x][0]
                    valid_move = False
                    if piece == "p" and brd[i,j] and brd[i,j][1] == enemy:
                        valid_move = True
                    elif piece == "k" and (brd[i,j] == None or brd[i,j][1] == enemy) and len(covers[i,j][enemy]) == 0:
                        valid_move = True
                    elif piece in ["n", "b", "r", "q"] and (brd[i,j] == None or brd[i,j][1] == enemy):
                        valid_move = True
    
                    if valid_move:
                        # check if the piece is pinned
                        if x in pinned_pieces and (i,j) in pinned_pieces_spans[x]:
                            #print(f"Pinned piece: {x}, span: {pinned_pieces_spans[x]}")
                            moves.append({"start":x, "end":(i,j), "piece":piece})
                        elif x not in pinned_pieces:
                            moves.append({"start":x, "end":(i,j), "piece":piece})

                for x in pawn_pushes[i,j][playing]:
                    if brd[i,j] is None:
                        if x in pinned_pieces and (i,j) in pinned_pieces_spans[x]:
                            moves.append({"start":x, "end":(i,j), "piece":"p"})
                        else:
                            moves.append({"start":x, "end":(i,j), "piece":"p"})

        # check for castling
        if can_castle:
            k = 0 if playing=="w" else 7
            if can_castle[playing]["short"] and len(covers[k,5][enemy])==0 and len(covers[k,6][enemy])==0 and brd[k,5]==None and brd[k,6]==None:
                moves.append({"special":"castle", "info":["short", playing]})
            if can_castle[playing]["long"] and len(covers[k,3][enemy])==0 and len(covers[k,2][enemy])==0 and brd[k,3]==None and brd[k,2]==None and brd[k,1]==None:
                moves.append({"special":"castle", "info":["long", playing]})
        
        # check if en passant can be made against the other player
        if enpassant:
            for x in enpassant["attackers"]:
                moves.append({"start":x, "end":enpassant["attacker_end"], "enpassant_execute":{"victim":enpassant["victim"]}})

    # go through and check for triggering en passant
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
    
    # go through moves and identify if any of them trigger draw
    
    return moves
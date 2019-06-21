import Data.List
-- The Poincaré disc is divided into 7-sided faces, with three faces meeting at
-- every vertex. Considering the point at the centre of a face, there are 7
-- triangles the face naturally divides into. Each triangle maps to an operator
-- in the group formed by composing operators A and D, both operators in SO(2,
-- 1). (The group, G, is a subgroup of SO(2, 1) homomorphic to the 7, 3, 2
-- Coxeter group.) Thus the disc is divided into triangles and each triangle
-- corresponds to some composition of As and Ds. A moves from one triangle to
-- another within a face and D jumps between the faces adjacent to the
-- designated vertex. Di is the inverse of D, etc.
data Op = A | Ai | D | Di
    deriving (Show)

data Face = I | Q | R
    deriving (Show)

-- Many sequences of A and D jumps will all lead to the same triangle. This
-- function computes the canonical representation of the operator arrived at by
-- composition of the operators in ops. Exactly one canonical representation
-- exists for every operator in G and thus for every triangle in the disc.
canonical :: [Op] -> [Op]
canonical ops = from I 0 ops
-- Canonical reps of operators starting from certain face types are possible,
-- as long as the operator doesn't stray from elements owned by the face. `from
-- f n rest` computes the canonical rep of the operator U = A**n*rest starting
-- from a face type f. If a canonical rep of U is impossible because U lands on
-- a triangle not owned by the root face of type f, it returns a non-canonical
-- rep of U that leads with Di, which can be used by the caller to figure out
-- what to do.
    where from f n []        = replicate n A
          from f n (op : rest) =
              case op of
                   A  -> from f ((n + 1) `mod` 7) rest
                   Ai -> from f ((n - 1) `mod` 7) rest
                   D  -> case f of
                              I -> resolve (from Q 0 rest)
                              _ -> case n of
                                        0 -> [Di,Di] ++ rest
                                        1 -> [Di,Di,Ai,Di] ++ rest
                                        2 -> resolve (from Q 0 rest)
                                        3 -> resolve (from f 0 rest)
                                        4 -> case f of
                                                  Q -> resolve (from R 0 rest)
                                                  R -> [Di,A,D,A,D,A] ++ rest
                                        5 -> [Di,A,D,A] ++ rest
                                        6 -> [Di,A] ++ rest
                         where resolve (Di : rest) = from f n rest
                               resolve canon     = replicate n A ++ D : canon
                   Di -> from f n ([Ai,D,Ai] ++ rest)

-- Put the operator represented by ops (As and Ds only) into n form. The AD
-- sequence is split into groups of As by the Ds, the As are counted modulo 7,
-- and the list of numbers is the result.
nform :: [Op] -> [Int]
nform ops = nnform 0 ops
-- nnform n ops puts the operator A**n*ops into n form.
    where nnform n [] = [n]
          nnform n (A : rest) = nnform ((n + 1) `mod` 7) rest
          nnform n (D : rest) = n : nnform 0 rest

-- Compose two operators in n form.
compn :: [Int] -> [Int] -> [Int]
compn [nl] (nr : rrest) = ((nl + nr) `mod` 7) : rrest
compn (nl : lrest) r = nl : compn lrest r

-- Convert an operator in n form to AD form.
adform :: [Int] -> Maybe [Op]
adform [] = Nothing
adform op = Just (f op)
    where f [n] = replicate (n `mod` 7) A
          f (n : rest) = replicate (n `mod` 7) A ++ [D] ++ f rest

-- A rewrite of canonical ops, but in n form. Maybe more efficient? Maybe a
-- little easier to read? I don't know, but there it is.
canonicaln :: [Int] -> [Int]
canonicaln ops = canon
    where (_, canon) = from I ops
    -- This from works the same as the other, except it returns (canonical?,
    -- ops), where canonical? is true iff ops is canonical. If canonical is
    -- false, a leading Di is implied.
          from f [n] = (True, [n])
          from f (n : rest) =
              case f of
                   I -> resolve (from Q rest)
                   _ -> case n of
                             0 -> (False, [6, 6] `compn` rest)
                             1 -> (False, [6, 4, 6] `compn` rest)
                             2 -> resolve (from Q rest)
                             3 -> resolve (from f rest)
                             4 -> case f of
                                       Q -> resolve (from R rest)
                                       R -> (False, [1, 1, 1] `compn` rest)
                             5 -> (False, [1, 1] `compn` rest)
                             6 -> (False, [1] `compn` rest)
              where resolve (isCanonical, ops) = if isCanonical
                                                 then (True, n : ops)
                                                 else from f ([n] `compn` ops)

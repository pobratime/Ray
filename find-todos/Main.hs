module Main (main) where

import Control.Monad 
import Data.List 
import System.Directory
import System.FilePath 
import Data.Char

sourceExtensions :: [String]
sourceExtensions = [".cpp", ".hpp"]

hasSourceExtension :: FilePath -> Bool
hasSourceExtension path = any (`isSuffixOf` path) sourceExtensions

collectFiles :: FilePath -> IO [FilePath]
collectFiles root = do
  entries <- listDirectory root
  let paths = map (root </>) entries
  files <- filterM doesFileExist paths
  dirs  <- filterM doesDirectoryExist paths
  nested <- forM dirs collectFiles
  return (files ++ concat nested)

data Hit = Hit FilePath Int String

trim :: String -> String
trim = dropWhileEnd isSpace . dropWhile isSpace

findTodosInFile :: FilePath -> IO [Hit]
findTodosInFile path = do
  contents <- readFile path
  let 
    numberedLines = zip [1 ..] $ lines contents
    hits = [ Hit path lineNo (trim line) | (lineNo, line) <- numberedLines, "TODO" `isInfixOf` line]
  return hits

printHit :: Hit -> IO ()
printHit (Hit file lineNo text) =
  putStrLn (file ++ " -> at line " ++ show lineNo ++ " -> " ++ text)

main :: IO ()
main = do
  let root = "../app"
  exists <- doesDirectoryExist root 
  if not exists
    then putStrLn $ "Directory not found: " ++ root
    else do
      allFiles <- collectFiles root
      let sourceFiles = filter hasSourceExtension allFiles
      allHits <- forM sourceFiles findTodosInFile
      let hits = concat allHits

      if null hits
        then putStrLn ("No TODOs found under " ++ root)
        else do
          putStrLn (show (length hits) ++ " TODO(s) found under " ++ root ++ ":")
          forM_ hits printHit


def CombinaisonGenerator(letters):
  if len(letters) == 0:
    return [[]]

  previousCombinaison = CombinaisonGenerator(letters[1:])
  nextCombinaison = []
  for elem in previousCombinaison:
    nextCombinaison.append(elem)

    newElem = elem[:]
    newElem.append(letters[0])
    nextCombinaison.append(newElem)
  
  return nextCombinaison


if __name__ == "__main__":
  import sys

  combinaison = CombinaisonGenerator(sys.argv[1:])
  for elem in combinaison[1:]:
    str = ""
    for subElem in elem:
      str += subElem
      str += "-"
    print str[:-1]

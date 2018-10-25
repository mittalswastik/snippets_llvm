#ifndef DF_H
#define DF_H

#include "Version.hh"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"


#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 5)
#include "llvm/IR/ValueMap.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/InstIterator.h"
#else
#include "llvm/ADT/ValueMap.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/InstIterator.h"
#endif

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <ostream>
#include <list>

using namespace llvm;

namespace {
typedef ValueMap<BasicBlock const *, BitVector *> DomainMap;

template <bool forward>
struct Dataflow {
  Dataflow() {
    in = new DomainMap();
    out = new DomainMap();
  }

  // in[b] where b is a basic block
  DomainMap *in;

  // out[b] where b is a basic block
  DomainMap *out;

  // bitvector such that meet(x, top) = x
  // must be specified by subclass
  BitVector *top;

  ~Dataflow() {
    for (DomainMap::value_type inEntry : *in) {
      delete inEntry.second;
    }
    for (DomainMap::value_type outEntry : *out) {
      delete outEntry.second;
    }
    delete top;
    delete in;  /// added
    delete out;
  }

  virtual bool runOnFunction(Function &f) {

    BasicBlock &entry = f.getEntryBlock();

    // initialize in and out
    for (BasicBlock const &BB : f) {
      if (forward) {
        /* Forward flow means we need to first apply meet with out[b]
           for all incoming blocks, b.

           With loops, out[b] may not always have been generated already,
           so we need initial interior points. */
        (*out)[&BB] = initialInteriorPoint(BB);

        /* just a dummy bitvector of the same length as top,
           it will never be read before being written to */
        (*in)[&BB] = new BitVector(*top);
      } else {
        // opposite logic for reverse flow
        (*in)[&BB] = initialInteriorPoint(BB);
        (*out)[&BB] = new BitVector(*top);

        // there isn't a unique exit node so we apply the boundary condition
        // when we reach a node with no successors in the loop below...
      }
    }

    if (forward) {
      // boundary conditions for entry node
      getBoundaryCondition((*in)[&entry]);
    }

    /* worklist maintains a list of all basic blocks on whom the transfer
     * function needs to be applied.*/
    auto worklist = new std::list<BasicBlock *>();

    // Initially, every node is in the worklist
    bfs(f, *worklist);

    if (!forward) {
      // for backward passes, start at exit nodes and work backwards
      worklist->reverse();
    }

    while (!worklist->empty()) {
      if (forward) {
        reversePostOrder(*worklist);
      } else {
        postOrder(*worklist);
      }
    }

    delete worklist;
    return false;
  }

  virtual void bfs(Function &f, std::list<BasicBlock *> &worklist) {
    BasicBlock *curNode;
    auto visited = new ValueMap<BasicBlock *, bool>();

    for (auto & elem : f) {
      (*visited)[&elem] = false;
    }

    auto l = new std::list<BasicBlock *>();

    l->push_back(&f.getEntryBlock());
    while (!l->empty()) {
      curNode = *(l->begin());
      l->pop_front();
      worklist.push_back(curNode);

      (*visited)[curNode] = true;
      for (succ_iterator SI = succ_begin(curNode), SE = succ_end(curNode);
           SI != SE; SI++) {
        if (!(*visited)[*SI]) {
          l->push_back(*SI);
        }
      }
    }

    delete visited;
    delete l;
  }

  virtual void reversePostOrder(std::list<BasicBlock *> &q) {
    BasicBlock *curNode = *q.begin();
    q.pop_front();

    pred_iterator PI = pred_begin(curNode), PE = pred_end(curNode);
    if (PI != PE) {
      // begin with a copy of out[first predecessor]
      *(*in)[curNode] = *(*out)[*PI];

      // fold meet over predecessors
      for (PI++; PI != PE; PI++) {
        meet((*in)[curNode], (*out)[*PI]);
      }
    }  // (otherwise entry node, in[entry] already set above)

    // apply transfer function
    BitVector *newOut = transfer(*curNode);
    if (*newOut != *(*out)[curNode]) {
      // copy new value
      *(*out)[curNode] = *newOut;
      for (succ_iterator SI = succ_begin(curNode), SE = succ_end(curNode);
           SI != SE; SI++) {
        q.push_back(*SI);
      }
    }
    delete newOut;
  }

  virtual void postOrder(std::list<BasicBlock *> &q) {
    BasicBlock *curNode = *q.begin();
    q.pop_front();

    succ_iterator SI = succ_begin(curNode), SE = succ_end(curNode);
    if (SI != SE) {
      // begin with a copy of in[first successor]
      *(*out)[curNode] = *(*in)[*SI];

      // fold meet operator over successors
      for (SI++; SI != SE; SI++) {
        meet((*out)[curNode], (*in)[*SI]);
      }
    } else {
      // boundary condition when it is an exit block
      getBoundaryCondition((*out)[curNode]);
    }

    // apply transfer function
    BitVector *newIn = transfer(*curNode);
    if (*newIn != *(*in)[curNode]) {
      // copy new value
      *(*in)[curNode] = *newIn;
      for (pred_iterator PI = pred_begin(curNode), PE = pred_end(curNode);
           PI != PE; PI++) {
        q.push_back(*PI);
      }
    }
    delete newIn;
  }

  virtual void getBoundaryCondition(BitVector *) = 0;
  virtual void meet(BitVector *, const BitVector *) = 0;
  virtual BitVector *initialInteriorPoint(BasicBlock const &) = 0;
  virtual BitVector *transfer(BasicBlock &) = 0;
};
}

#endif
#include <iostream>
#include <cmath>
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/raw_os_ostream.h"

// Module
//   @name : string
//   Type TODO
//     @name : string TODO
//     @type : string TODO
//     @operand : ref TODO
//   Metadata TODO
//     @type : string TODO
//     @value : string TODO
//     @operand : ref TODO
//   NamedMetadata TODO
//     @name : string TODO
//     @metadata : ref TODO
//   GlobalVariable
//     @name : string
//     Type
//     Use
//   Function
//     @name : string
//     Type
//     Use
//     BasicBlock
//       @name : string
//       Type
//       Use
//       Instruction
//         @name : string
//         @operand# : ref
//         @metadata# : ref TODO
//         Type
//         Constant
//         Use
//           @user : ref
//           @operand : int

namespace pugi
{
  std::vector<std::string> NumbersStrings;
  std::vector<std::string> OperandStrings;
  std::vector<std::string> MetadataStrings;
  std::map<const void *, std::string> IDCache;
  unsigned NextID = 0;

  class xml_node_struct_ref;

	class xml_attribute_struct_ref {
    public:
      enum Type {
        ID,
        Name,
        Operand,
        Metadata,
        OperandNo,
        User,
        End
      };

    private:
      Type T;
      unsigned Index;
      const xml_node_struct_ref *ref;

	public:
      xml_attribute_struct_ref() {
        *this = null();
      }

      xml_attribute_struct_ref(const xml_node_struct_ref *ref);

		static xml_attribute_struct_ref null() {
          xml_attribute_struct_ref Result(nullptr);
          Result.T = End;
          return Result;
		}

    private:
      static std::string getName(llvm::Value *V) {
        if (V->hasName())
          return V->getName();
        else
          return std::to_string(NextID++);
      }

      static char_t *getID(llvm::Value *P) {
        auto It = IDCache.find(P);
        if (It == IDCache.end()) {
          std::string ID;
          if (auto *I = llvm::dyn_cast<llvm::Instruction>(P))
            ID = getID(I->getParent()) + ("_" + getName(P));
          else if (auto *I = llvm::dyn_cast<llvm::BasicBlock>(P))
            ID = getID(I->getParent()) + ("_" + getName(P));
          else if (auto *F = llvm::dyn_cast<llvm::GlobalObject>(P))
            ID = getName(P);
          else if (auto *C = llvm::dyn_cast<llvm::Constant>(P)) {
            llvm::raw_string_ostream Stream(ID);
            C->printAsOperand(Stream, false);
          } else
            ID = getName(P);

          It = IDCache.insert({ P, ID }).first;
        }

        return const_cast<char_t*>(It->second.data());
      }

    public:

      bool operator==(const xml_attribute_struct_ref& Other) const {
        return std::tie(T, Index, ref) == std::tie(Other.T, Other.Index, Other.ref);
      }

      bool operator!=(const xml_attribute_struct_ref& Other) const {
        return !(*this == Other);
      }

		operator bool() const {
          return T != End;
		}

		char_t*	name() const
		{
          switch (T) {
          case User:
            return const_cast<char_t*>("user");
          case OperandNo:
            return const_cast<char_t*>("operand");
          case ID:
            return const_cast<char_t*>("xml:id");
          case Name:
            return const_cast<char_t*>("name");
          case Operand:
            if (OperandStrings.size() <= Index)
              OperandStrings.push_back("operand" + std::to_string(Index));
            return const_cast<char_t*>(OperandStrings[Index].c_str());
          case Metadata:
            if (MetadataStrings.size() <= Index)
              MetadataStrings.push_back("metadata" + std::to_string(Index));
            return const_cast<char_t*>(MetadataStrings[Index].c_str());
          }
		}

      char_t* value() const;

      xml_attribute_struct_ref prev_attribute_c() const
      {
        assert(false);
      }

      xml_attribute_struct_ref next_attribute() const;

	};

	class xml_node_struct_ref {
    private:
      friend class xml_attribute_struct_ref;

      enum Type {
        Null,
        Module,
        Value,
        Use,
        TheType,
        TypeList
      };

      Type T;
      llvm::Value *V;
      llvm::Module *M;
      llvm::Use *U;

    private:
      llvm::Module *getModule() const {
        assert(T == Module || T == TypeList); return M;
      }
      llvm::Value *getValue() const { assert(T == Value); return V; }
      llvm::Use *getUse() const { assert(T == Use); return U; }
      llvm::Type *getType() const { assert(T == TheType); return V->getType(); }
      llvm::Value *getOwner() const { assert(T == TheType); return V; }

    public:
      // TODO: replace with factory methods
      xml_node_struct_ref(llvm::Module *M) : T(Module), V(nullptr), M(M), U(nullptr) { }
      xml_node_struct_ref(llvm::Value *V) : T(Value), V(V), M(nullptr), U(nullptr) { }
      xml_node_struct_ref(llvm::Use *U) : T(Use), V(nullptr), M(nullptr), U(U) { }
    public:
      xml_node_struct_ref() : T(Null), V(nullptr), M(nullptr), U(nullptr) { }

      static xml_node_struct_ref createType(llvm::Value *Owner) {
        xml_node_struct_ref Result;
        Result.T = TheType;
        Result.V = Owner;
        return Result;
      }

      static xml_node_struct_ref createTypeList(llvm::Module *Module) {
        xml_node_struct_ref Result;
        Result.T = TypeList;
        Result.M = Module;
        return Result;
      }

      bool operator==(const xml_node_struct_ref& Other) const {
        return std::tie(T, V, M, U) == std::tie(Other.T, Other.V, Other.M, Other.U);
      }

      bool operator!=(const xml_node_struct_ref& Other) const {
        return !(*this == Other);
      }

      void dump(llvm::raw_os_ostream &Output) {
        switch (T) {
        case Module:
          getModule()->print(Output, nullptr, false, true);
          break;
        case Value:
          getValue()->print(Output, true);
          break;
        case Use:
          llvm::Use *TheUse = getUse();
          Output << "Use " << TheUse->getOperandNo() << " of ";
          getUse()->getUser()->print(Output, true);
          Output << ": ";
          getUse()->get()->print(Output, true);
        }
      }

      const char_t *name() const {
        switch (T) {
        case Module:
          return "module";
          break;
        case Value:
          if (auto *I = llvm::dyn_cast<llvm::Instruction>(getValue()))
            return I->getOpcodeName();
          else if (auto *I = llvm::dyn_cast<llvm::BasicBlock>(getValue()))
            return "basicblock";
          else if (auto *F = llvm::dyn_cast<llvm::Function>(getValue()))
            return "function";
          else if (auto *GV = llvm::dyn_cast<llvm::GlobalVariable>(getValue()))
            return "global";
          else
            assert(false);
          break;
        case Use:
          return "use";
        case TheType:
          return "type";
        case TypeList:
          return "types";
        }

        return nullptr;
      }

      char_t *value() const {
        return const_cast<char_t*>("");
      }

      xml_node_struct_ref parent() const {
        switch (T) {
        case Module:
          break;
        case Value:
          if (auto *I = llvm::dyn_cast<llvm::Instruction>(getValue())) {
            return xml_node_struct_ref(I->getParent());
          } else if (auto *BB = llvm::dyn_cast<llvm::BasicBlock>(getValue())) {
            return xml_node_struct_ref(BB->getParent());
          } else if (auto *F = llvm::dyn_cast<llvm::Function>(getValue())) {
            return xml_node_struct_ref(F->getParent());
          }

          break;
        case Use:
          return xml_node_struct_ref(getUse()->get());
        case TheType:
          return xml_node_struct_ref(getOwner());
        case TypeList:
          return root();
        }

        return xml_node_struct_ref();
      }

      xml_node_struct_ref next_child(const xml_node_struct_ref *Last) const {
        switch (T) {
        case Module:
          {
            bool GlobalsDone = false;
            bool FunctionsDone = false;

            if (Last == nullptr) {
              return xml_node_struct_ref::createTypeList(getModule());
            } else {
              if (Last->T == TypeList) {
                auto It = getModule()->global_begin();
                if (It != getModule()->global_end())
                  return xml_node_struct_ref(&*It);
                else
                  GlobalsDone = true;
              } else if (Last->T == Value) {
                if (auto *GV = llvm::dyn_cast<llvm::GlobalVariable>(Last->getValue())) {
                  auto It = GV->getIterator();
                  It++;
                  if (It != GV->getParent()->global_end())
                    return xml_node_struct_ref(&*It);
                  else
                    GlobalsDone = true;
                } else if (auto *F = llvm::dyn_cast<llvm::Function>(Last->getValue())) {
                  auto It = F->getIterator();
                  It++;
                  if (It != F->getParent()->end())
                    return xml_node_struct_ref(&*It);
                  else
                    FunctionsDone = true;
                }
              }
            }

            if (GlobalsDone) {
              // Start with functions
              auto It = getModule()->begin();
              if (It != getModule()->end())
                return xml_node_struct_ref(&*It);
              else
                FunctionsDone = true;
            }

            if (FunctionsDone) {
              // Terminate
              return xml_node_struct_ref();
            }

            assert(false);


            if (Last == nullptr) {
              auto It = getModule()->begin();
              if (It != getModule()->end())
                return xml_node_struct_ref(&*It);
            } else {
              assert(Last->T == Value);
              auto *LastFunction = llvm::cast<llvm::Function>(Last->getValue());
              auto It = LastFunction->getIterator();
              It++;
              if (It != LastFunction->getParent()->end())
                return xml_node_struct_ref(&*It);
            }

            break;
          }
        case Value:
          {
            // Possible actions:
            //
            // * Start with uses: Last == nullptr
            // * Continue with uses: (Last != nullptr && Last->T == Use)
            // * Start with child values: (Last == nullptr && UsesDone)
            // * Continue with childe values: (Last != nullptr && Last->T == Value)
            // * Terminate: (Last == nullptr || UsesDone && ValuesDone

            bool TypeDone = false;
            bool UsesDone = false;
            bool ValuesDone = false;

            if (Last == nullptr) {
              return xml_node_struct_ref::createType(getValue());
            } else {
              if (Last->T == TheType) {
                auto It = getValue()->use_begin();
                if (It != getValue()->use_end())
                  return xml_node_struct_ref(&*It);
                else
                  UsesDone = true;
              } else if (Last->T == Use) {
                // Try to continue with use or, if you're done, switch to
                // value, or terminate
                if (llvm::Use *Next = Last->getUse()->getNext()) {
                  return xml_node_struct_ref(Next);
                } else {
                  UsesDone = true;
                }
              } else if (Last->T == Value) {
                // Continue with value or terminate
                if (auto *I = llvm::dyn_cast<llvm::Instruction>(Last->getValue())) {
                  auto It = I->getIterator();
                  It++;
                  if (It != I->getParent()->end())
                    return xml_node_struct_ref(&*It);
                  else
                    ValuesDone = true;
                } else if (auto *BB = llvm::dyn_cast<llvm::BasicBlock>(Last->getValue())) {
                  auto It = BB->getIterator();
                  It++;
                  if (It != BB->getParent()->end())
                    return xml_node_struct_ref(&*It);
                  else
                    ValuesDone = true;
                } else if (auto *F = llvm::dyn_cast<llvm::Function>(Last->getValue())) {
                  auto It = F->getIterator();
                  It++;
                  if (It != F->getParent()->end())
                    return xml_node_struct_ref(&*It);
                  else
                    ValuesDone = true;
                }

              }
            }

            if (UsesDone) {
              // Start with child values
              if (auto *BB = llvm::dyn_cast<llvm::BasicBlock>(getValue())) {
                auto It = BB->begin();
                if (It != BB->end())
                  return xml_node_struct_ref(&*It);
                else
                  ValuesDone = true;
              } else if (auto *F = llvm::dyn_cast<llvm::Function>(getValue())) {
                auto It = F->begin();
                if (It != F->end())
                  return xml_node_struct_ref(&*It);
                else
                  ValuesDone = true;
              } else if (auto *I = llvm::dyn_cast<llvm::Instruction>(getValue())) {
                ValuesDone = true;
              } else if (auto *I = llvm::dyn_cast<llvm::GlobalVariable>(getValue())) {
                ValuesDone = true;
              } else {
                assert(false);
              }
            }

            if (ValuesDone) {
              // Terminate
              return xml_node_struct_ref();
            }

            assert(false);
          }

          break;
        case Use:
          break;
        case TheType:
          break;
        case TypeList:
          break;
        }

        return xml_node_struct_ref();
      }

      xml_node_struct_ref first_child() const {
        return next_child(nullptr);

        // switch (T) {
        // case Module:
        //   {
        //     auto It = getModule()->begin();
        //     if (It != getModule()->end())
        //       return xml_node_struct_ref(&*It);
        //     break;
        //   }
        // case Value:
        //   if (auto *BB = llvm::dyn_cast<llvm::BasicBlock>(getValue())) {
        //     auto It = BB->begin();
        //     if (It != BB->end())
        //       return xml_node_struct_ref(&*It);
        //   } else if (auto *F = llvm::dyn_cast<llvm::Function>(getValue())) {
        //     auto It = F->begin();
        //     if (It != F->end())
        //       return xml_node_struct_ref(&*It);
        //   } else if (auto *I = llvm::dyn_cast<llvm::Instruction>(getValue())) {
        //     // auto It = I->begin();
        //     // if (It != I->end())
        //     //   return xml_node_struct_ref(&*It);
        //   }

        //   break;
        // case Use:
        //   break;
        // }

        // return xml_node_struct_ref();
      }

      xml_node_struct_ref prev_sibling_c() const {
        // switch (T) {
        // case Module:
        //   break;
        // case Value:
        //   if (auto *I = llvm::dyn_cast<llvm::Instruction>(getValue())) {
        //     auto It = I->getIterator();
        //     if (It == I->getParent()->begin()) {
        //       return xml_node_struct_ref(&*I->getParent()->rbegin());
        //     } else {
        //       return xml_node_struct_ref(&*--It);
        //     }
        //   } else if (auto *BB = llvm::dyn_cast<llvm::BasicBlock>(getValue())) {
        //     auto It = BB->getIterator();
        //     if (It == BB->getParent()->begin()) {
        //       return xml_node_struct_ref(&BB->getParent()->back());
        //     } else {
        //       return xml_node_struct_ref(&*--It);
        //     }
        //   } else if (auto *F = llvm::dyn_cast<llvm::Function>(getValue())) {
        //     auto It = F->getIterator();
        //     if (It == F->getParent()->begin()) {
        //       return xml_node_struct_ref(&*F->getParent()->rbegin());
        //     } else {
        //       return xml_node_struct_ref(&*--It);
        //     }
        //   }

        //   break;
        // case Use:
        //   auto It = getUse()->getOperandNo();
        //   llvm::User *U = getUse()->getUser();
        //   if (It == 0) {
        //     return xml_node_struct_ref(U->getOperand(U->getNumOperands() - 1));
        //   } else {
        //     return xml_node_struct_ref(U->getOperand(--It));
        //   }
        //   break;
        // }

        // return xml_node_struct_ref();

        assert(false);
      }

      xml_node_struct_ref next_sibling() const {
        switch (T) {
        case Module:
          break;
        case Value:
          if (auto *I = llvm::dyn_cast<llvm::Instruction>(getValue())) {
            return xml_node_struct_ref(I->getParent()).next_child(this);
          } else if (auto *BB = llvm::dyn_cast<llvm::BasicBlock>(getValue())) {
            return xml_node_struct_ref(BB->getParent()).next_child(this);
          } else if (auto *F = llvm::dyn_cast<llvm::Function>(getValue())) {
            return xml_node_struct_ref(F->getParent()).next_child(this);
          } else if (auto *GV = llvm::dyn_cast<llvm::GlobalVariable>(getValue())) {
            return xml_node_struct_ref(GV->getParent()).next_child(this);
          } else {
            assert(false);
          }
        case Use:
          return xml_node_struct_ref(getUse()->get()).next_child(this);
        case TheType:
          return xml_node_struct_ref(getOwner()).next_child(this);
        case TypeList:
          return root().next_child(this);
        }

        assert(false);

        // switch (T) {
        // case Module:
        //   break;
        // case Value:
        //   if (auto *I = llvm::dyn_cast<llvm::Instruction>(getValue())) {
        //     auto It = I->getIterator();
        //     It++;
        //     if (It != I->getParent()->end())
        //       return xml_node_struct_ref(&*It);
        //   } else if (auto *BB = llvm::dyn_cast<llvm::BasicBlock>(getValue())) {
        //     auto It = BB->getIterator();
        //     It++;
        //     if (It != BB->getParent()->end())
        //       return xml_node_struct_ref(&*It);
        //   } else if (auto *F = llvm::dyn_cast<llvm::Function>(getValue())) {
        //     auto It = F->getIterator();
        //     It++;
        //     if (It != F->getParent()->end())
        //       return xml_node_struct_ref(&*It);
        //   }

        //   break;
        // case Use:
        //   auto It = getUse()->getNext();
        //   if (It != nullptr)
        //     return xml_node_struct_ref(It);
        //   break;
        // }

        // return xml_node_struct_ref();
      }

      xml_node_struct_ref root() const {
        switch (T) {
        case Module:
          return *this;
        case Value:
          if (auto *I = llvm::dyn_cast<llvm::Instruction>(getValue())) {
            return xml_node_struct_ref(I->getParent()).root();
          } else if (auto *BB = llvm::dyn_cast<llvm::BasicBlock>(getValue())) {
            return xml_node_struct_ref(BB->getParent()).root();
          } else if (auto *F = llvm::dyn_cast<llvm::Function>(getValue())) {
            return xml_node_struct_ref(F->getParent()).root();
          }
        case Use:
          return xml_node_struct_ref(getUse()->getUser()).root();
        case TheType:
          return xml_node_struct_ref(getOwner()).root();
        case TypeList:
          return xml_node_struct_ref(getModule());
        }

        assert(false);
      }

		static xml_node_struct_ref null() {
          xml_node_struct_ref Result;
          return Result;
		}

		operator bool() const {
          return T != Null;
		}

		xml_node_type type() const {
          return node_element;
		}

		xml_attribute_struct_ref first_attribute() const {
          return xml_attribute_struct_ref(this);
		}

	};

  xml_attribute_struct_ref xml_attribute_struct_ref::next_attribute() const
  {
    xml_attribute_struct_ref Result = null();
    switch (ref->T) {
    case xml_node_struct_ref::Null:
      assert(false);
      break;
    case xml_node_struct_ref::Module:
      if (T == ID) {
        Result = *this;
        Result.T = Name;
      }
      break;
    case xml_node_struct_ref::Value:
      // Users have operands
      switch (T) {
      case ID:
        Result = *this;
        Result.T = Name;
        break;
      case Name:
        if (auto *U = llvm::dyn_cast<llvm::User>(ref->V)) {
          if (U->getNumOperands() > 0) {
            Result = *this;
            Result.T = Operand;
            Result.Index = 0;
            // } else if (auto *I = llvm::dyn_cast<llvm::Instruction>(ref->V)) {
            //   if (I->hasMetadata()) {
            //     T = Metadata;
            //     Index = 0;
            //   } else {
            //   }
          }
        }
        break;
      case Operand:
        if (auto *U = llvm::dyn_cast<llvm::User>(ref->V)) {
          if (Index + 1 < U->getNumOperands()) {
            Result = *this;
            Result.Index++;
          }
        }
        break;
      case Metadata:
      case End:
        assert(false);
        break;
      }

      break;
    case xml_node_struct_ref::Use:
      switch (T) {
      case User:
        Result = *this;
        Result.T = OperandNo;
        break;
      }
      break;
    }

    return Result;
  }

  char_t* xml_attribute_struct_ref::value() const
  {
    switch (T) {
    case User:
      assert(ref->T == xml_node_struct_ref::Use);
      return getID(ref->getUse()->getUser());
    case OperandNo:
      {
        assert(ref->T == xml_node_struct_ref::Use);
        auto Index = ref->getUse()->getOperandNo();
        if (NumbersStrings.size() <= Index)
          NumbersStrings.push_back(std::to_string(Index));
        return const_cast<char_t*>(NumbersStrings[Index].c_str());
      }
    case ID:
      {
        switch (ref->T) {
        case xml_node_struct_ref::Null:
          return const_cast<char_t*>("null");
        case xml_node_struct_ref::Module:
          return const_cast<char_t*>("module");
        case xml_node_struct_ref::Value:
          return getID(ref->getValue());
        case xml_node_struct_ref::Use:
          assert(false);
          break;
        }
      }
    case Name:
      switch (ref->T) {
      case xml_node_struct_ref::Module:
        return const_cast<char_t*>(ref->getModule()->getName().data());
      case xml_node_struct_ref::Value:
        return const_cast<char_t*>(ref->getValue()->getName().data());
      }

      break;
    case Operand:
      {
        llvm::Value *Operand = llvm::cast<llvm::User>(ref->getValue())->getOperand(Index);
        return getID(Operand);
      }
      break;
    case Metadata:
      break;
    }

    return const_cast<char_t*>("");
  }

  xml_attribute_struct_ref::xml_attribute_struct_ref(const xml_node_struct_ref *ref) : ref(ref) {
    if (ref != nullptr && ref->T == xml_node_struct_ref::Use)
      T = User;
    else
      T = ID;
  }

}

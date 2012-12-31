#include "Tracing.h"
#include "IRMutator.h"

namespace Halide {
namespace Internal {

class InjectTracing : public IRMutator {
public:
    int level;
    InjectTracing() {
        char *trace = getenv("HL_TRACE");
        level = trace ? atoi(trace) : 0;
    }


private:
    void visit(const Call *op) {
        expr = op;
    }

    void visit(const Provide *op) {       
        IRMutator::visit(op);
        if (level >= 2) {
            const Provide *op = stmt.as<Provide>();
            vector<Expr> args = op->args;
            args.push_back(op->value);
            Stmt print = new PrintStmt("Provide " + op->buffer, args);
            stmt = new Block(print, op);
        }
    }

    void visit(const Realize *op) {
        IRMutator::visit(op);
        if (level >= 1) {
            const Realize *op = stmt.as<Realize>();
            vector<Expr> args;
            for (size_t i = 0; i < op->bounds.size(); i++) {
                args.push_back(op->bounds[i].first);
                args.push_back(op->bounds[i].second);
            }
            Stmt print = new PrintStmt("Realizing " + op->buffer + " over ", args);
            Stmt body = new Block(print, op->body);
            stmt = new Realize(op->buffer, op->type, op->bounds, body);
        }        
    }
};

Stmt inject_tracing(Stmt s) {
    return InjectTracing().mutate(s);
}

}
}
